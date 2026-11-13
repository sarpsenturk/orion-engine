#include "renderer.hpp"

#include "orion/core/assert.hpp"
#include "orion/core/logger.hpp"
#include "orion/core/window.hpp"
#include "orion/utils/array.hpp"
#include "orion/utils/scope_guard.hpp"
#include "orion/utils/timer.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <utility>

namespace orion
{
    Renderer::Renderer()
        : m_device(gpu::create_device(gpu::DeviceDesc{}), gpu::destroy_device)
        , m_frame_data(make_array<frames_in_flight>([this] { return Frame{m_device.get()}; }))
        , m_mesh_manager(m_device.get())
        , m_material_manager(m_device.get())
        , m_texture_manager(m_device.get())

    {
    }

    Renderer::~Renderer()
    {
        // Wait until all work on the GPU has finished before tearing down the Renderer
        gpu::wait_idle(m_device.get());

        for (auto& frame : m_frame_data) {
            frame.m_deletion_queue.flush();
        }
        m_pending_deletions.flush();

        for (auto [_, swapchain] : m_swapchains) {
            gpu::destroy_swapchain(m_device.get(), swapchain);
        }

        if (m_depth_image_view) { gpu::destroy_image_view(m_device.get(), m_depth_image_view); }
        if (m_depth_image) { gpu::destroy_image(m_device.get(), m_depth_image); }
    }

    Frame* Renderer::begin_frame(gpu::Swapchain* swapchain)
    {
        m_current_frame_stats = FrameStats{.frame_index = m_frame_counter};
        auto& frame_data      = current_frame();
        // Wait until the previous submission of this frame has finished
        const auto wait_timer = Timer{};
        gpu::wait_timeline(m_device.get(), frame_data.m_timeline_point);
        frame_data.reset();
        m_current_frame_stats.frame_wait_ms = wait_timer.elapsed();
        frame_data.m_deletion_queue.flush();
        // Reset the command allocator for this frame
        gpu::reset_command_allocator(m_device.get(), frame_data.m_command_allocator);
        // Acquire the next frame from the swapchain
        // Return the active Frame if successful, otherwise return nullptr
        const auto acquire_timer                   = Timer{};
        auto swapchain_frame                       = gpu::acquire_swapchain_frame(m_device.get(), swapchain);
        m_current_frame_stats.swapchain_acquire_ms = acquire_timer.elapsed();
        if (swapchain_frame) {
            const auto swapchain_extent = gpu::image_extent_2d(swapchain_frame->image);
            if (m_depth_image) {
                const auto depth_extent = gpu::image_extent_2d(m_depth_image);
                if (depth_extent.width != swapchain_extent.width || depth_extent.height != swapchain_extent.height) {
                    m_pending_deletions.push(
                        [device = m_device.get(), image_view = m_depth_image_view, image = m_depth_image] {
                            gpu::destroy_image_view(device, image_view);
                            gpu::destroy_image(device, image);
                        });
                    m_depth_image_view = nullptr;
                    m_depth_image      = nullptr;
                }
            }
            // TODO: Replace with a transient render graph image.
            if (!m_depth_image) {
                m_depth_image = gpu::create_image(
                    m_device.get(),
                    gpu::ImageDesc{
                        .width  = swapchain_extent.width,
                        .height = swapchain_extent.height,
                        .format = opaque_depth_format,
                        .usage  = gpu::ImageUsage::DepthStencilAttachment,
                    });
                m_depth_image_view = gpu::create_image_view(
                    m_device.get(),
                    gpu::ImageViewDesc{
                        .image = m_depth_image,
                    });
            }

            frame_data.m_swapchain_frame = *swapchain_frame;
            frame_data.m_command_buffer  = gpu::begin_command_buffer(m_device.get(), frame_data.m_command_allocator);
            return &frame_data;
        } else {
            ORION_LOG_WARN(LogChannel::Renderer, "Swapchain frame acquisition failed, possibly due to window resize or minimization");
            return nullptr;
        }
    }

    void Renderer::render_view(Frame& frame, ViewIndex index)
    {
        const auto& render_view = frame.view(index);
        opaque_pass(frame, render_view);
        if (render_view.imgui_draw_data) {
            imgui_pass(frame, render_view.imgui_draw_data);
        }
    }

    void Renderer::opaque_pass(Frame& frame, const RenderView& render_view)
    {
        const auto record_timer = Timer{};
        auto* command_buffer    = frame.m_command_buffer;

        // Transition the swapchain image to color attachment optimal layout
        // Transition the depth image to depth attachment optimal layout
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image             = frame.m_swapchain_frame.image,
                        .src_stages        = gpu::PipelineStage::ColorAttachmentOutput,
                        .src_access        = gpu::Access::None,
                        .dst_stages        = gpu::PipelineStage::ColorAttachmentOutput,
                        .dst_access        = gpu::Access::ColorAttachmentWrite,
                        .old_layout        = gpu::ImageLayout::Undefined,
                        .new_layout        = gpu::ImageLayout::ColorAttachment,
                        .subresource_range = {
                            .base_mip_level   = 0u,
                            .level_count      = 1u,
                            .base_array_layer = 0u,
                            .layer_count      = 1u,
                        },
                    },
                    gpu::ImageBarrier{
                        .image             = m_depth_image,
                        .src_stages        = gpu::PipelineStage::DepthStencilTests,
                        .src_access        = gpu::Access::DepthStencilAttachmentWrite,
                        .dst_stages        = gpu::PipelineStage::DepthStencilTests,
                        .dst_access        = gpu::Access::DepthStencilAttachmentWrite,
                        .old_layout        = gpu::ImageLayout::Undefined,
                        .new_layout        = gpu::ImageLayout::DepthStencilAttachment,
                        .subresource_range = {
                            .base_mip_level   = 0u,
                            .level_count      = 1u,
                            .base_array_layer = 0u,
                            .layer_count      = 1u,
                        },
                    },
                }},
            });
        // Begin render pass
        gpu::begin_render_pass(
            command_buffer,
            gpu::RenderPass{
                .color_attachments = {{
                    gpu::RenderPassAttachment{
                        .image_view = frame.m_swapchain_frame.image_view,
                        .load_op    = gpu::LoadOp::Clear,
                        .store_op   = gpu::StoreOp::Store,
                    },
                }},
                .depth_attachment  = gpu::RenderPassAttachment{
                    .image_view = m_depth_image_view,
                    .load_op    = gpu::LoadOp::Clear,
                    .store_op   = gpu::StoreOp::Store,
                },
            });
        gpu::bind_sampler_heap(command_buffer, m_texture_manager.sampler_heap());
        gpu::bind_resource_heap(command_buffer, m_texture_manager.resource_heap());
        gpu::set_viewport(command_buffer, render_view.viewport);
        gpu::set_scissor(command_buffer, render_view.scissor);
        for (const auto& packet : render_view.opaque_draws) {
            gpu::bind_pipeline_state(command_buffer, packet.pipeline);
            gpu::bind_index_buffer(command_buffer, packet.index_buffer, packet.index_buffer_offset);
            gpu::draw_indexed(command_buffer, packet.draw, packet.root());
            m_current_frame_stats.instance_count += packet.draw.instance_count;
            ++m_current_frame_stats.draw_calls;
            m_current_frame_stats.index_count += packet.draw.index_count * packet.draw.instance_count;
            m_current_frame_stats.triangle_count += packet.draw.index_count / 3u * packet.draw.instance_count;
        }
        // End render pass
        gpu::end_render_pass(command_buffer);
        m_current_frame_stats.opaque_record_ms += record_timer.elapsed();
    }

    void Renderer::imgui_pass(Frame& frame, ImDrawData* draw_data)
    {
        const auto record_timer = Timer{};
        auto* command_buffer    = frame.m_command_buffer;

        // ImGui render pass
        gpu::begin_render_pass(
            command_buffer,
            gpu::RenderPass{
                .color_attachments = {{
                    gpu::RenderPassAttachment{
                        .image_view = frame.m_swapchain_frame.image_view,
                        .load_op    = gpu::LoadOp::Load,
                        .store_op   = gpu::StoreOp::Store,
                    },
                }},
            });
        ImGui_ImplOrion_RenderDrawData(draw_data, command_buffer);
        gpu::end_render_pass(command_buffer);
        m_current_frame_stats.ui_record_ms += record_timer.elapsed();
    }

    void Renderer::end_frame(Frame& frame)
    {
        auto* command_buffer = frame.m_command_buffer;

        // Transition the swapchain image to present source layout
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image             = frame.m_swapchain_frame.image,
                        .src_stages        = gpu::PipelineStage::ColorAttachmentOutput,
                        .src_access        = gpu::Access::ColorAttachmentWrite,
                        .dst_stages        = gpu::PipelineStage::AllCommands,
                        .dst_access        = gpu::Access::None,
                        .old_layout        = gpu::ImageLayout::ColorAttachment,
                        .new_layout        = gpu::ImageLayout::Present,
                        .subresource_range = {
                            .base_mip_level   = 0u,
                            .level_count      = 1u,
                            .base_array_layer = 0u,
                            .layer_count      = 1u,
                        },
                    },
                }},
            });

        // Submit the command buffers to the GPU
        const auto submit_timer = Timer{};
        frame.m_timeline_point  = gpu::submit(
            m_device.get(),
            gpu::Queue::Graphics,
            gpu::Submit{
                .command_buffers = {{command_buffer}},
                .swapchain_frame = &frame.m_swapchain_frame,
            });
        m_current_frame_stats.command_submit_ms = submit_timer.elapsed();

        // Retire pending resources once this submission has completed. The frame
        // queue was flushed in begin_frame(), leaving the pending queue empty.
        std::swap(m_pending_deletions, frame.m_deletion_queue);

        // Present the swapchain image
        const auto present_timer = Timer{};
        gpu::present(m_device.get(), frame.m_swapchain_frame);
        m_current_frame_stats.swapchain_present_ms = present_timer.elapsed();
        m_current_frame_stats.allocation_bytes     = frame.allocation_bytes();
        m_current_frame_stats.used_bytes           = frame.used_bytes();
        m_last_frame_stats                         = m_current_frame_stats;
        // Increment the frame counter
        ++m_frame_counter;
    }

    gpu::Swapchain* Renderer::create_swapchain(const Window& window)
    {
        ORION_ASSERT(!m_swapchains.contains(&window) && "A swapchain for this window already exists");

        auto* swapchain = gpu::create_swapchain(
            m_device.get(),
            gpu::SwapchainDesc{
                .native_window_handle = window.handle(),
                .width                = static_cast<std::uint32_t>(window.width()),
                .height               = static_cast<std::uint32_t>(window.height()),
                .image_count          = swapchain_image_count,
                .image_format         = swapchain_image_format,
            });
        m_swapchains.insert(std::make_pair(&window, swapchain));
        return swapchain;
    }

    void Renderer::on_event(const Event& event)
    {
        if (const auto* window_resize = std::get_if<OnWindowResize>(&event)) {
            // If a swapchain for this window exists, resize it
            if (auto it = m_swapchains.find(window_resize->window); it != m_swapchains.end()) {
                // Skip resizing if the new dimensions are zero (minimized window)
                if (window_resize->width == 0 || window_resize->height == 0) { return; }

                // Wait until the GPU is idle before resizing the swapchain
                gpu::wait_idle(m_device.get());
                gpu::resize_swapchain(m_device.get(), it->second, window_resize->width, window_resize->height);
            }
        }
    }

    RendererStats Renderer::renderer_stats() const noexcept
    {
        return RendererStats{
            .mesh_count                   = m_mesh_manager.mesh_count(),
            .material_count               = m_material_manager.material_count(),
            .texture_count                = m_texture_manager.texture_count(),
            .mesh_vertex_used_bytes       = m_mesh_manager.vertex_used_bytes(),
            .mesh_index_used_bytes        = m_mesh_manager.index_used_bytes(),
            .mesh_vertex_allocation_bytes = m_mesh_manager.vertex_allocation_bytes(),
            .mesh_index_allocation_bytes  = m_mesh_manager.index_allocation_bytes(),
            .mesh_record_allocation_bytes = m_mesh_manager.record_allocation_bytes(),
            .material_allocation_bytes    = m_material_manager.allocation_bytes(),
            .texture_allocation_bytes     = m_texture_manager.allocation_bytes(),
        };
    }

    Mesh Renderer::create_mesh(const MeshDesc& desc)
    {
        return m_mesh_manager.create_mesh(desc);
    }

    void Renderer::destroy_mesh(Mesh mesh)
    {
        m_pending_deletions.push([this, mesh] {
            m_mesh_manager.destroy_mesh(mesh);
        });
    }

    Material Renderer::create_material(const PbrMaterial& material)
    {
        ORION_ASSERT(material.base_color_texture.handle == Texture{}.handle ||
                     m_texture_manager.get_image(material.base_color_texture) != nullptr);
        return m_material_manager.create_material(material);
    }

    void Renderer::destroy_material(Material material)
    {
        m_pending_deletions.push([this, material] {
            m_material_manager.destroy_material(material);
        });
    }

    Texture Renderer::create_texture(const TextureDesc& desc)
    {
        ORION_ASSERT(desc.width != 0u);
        ORION_ASSERT(desc.height != 0u);
        ORION_ASSERT(desc.data.data != nullptr);
        ORION_ASSERT(desc.data.size % 4z == 0z);
        ORION_ASSERT(desc.data.size / 4z % desc.width == 0z);
        ORION_ASSERT(desc.data.size / 4z / desc.width == desc.height);

        // Create texture object
        const auto texture = m_texture_manager.create_texture(desc);

        const auto texture_guard = ScopeFail{[&] { m_texture_manager.destroy_texture(texture); }};
        auto* image              = m_texture_manager.get_image(texture);

        auto* staging_buffer = gpu::create_buffer(
            m_device.get(),
            gpu::BufferDesc{
                .size   = desc.data.size,
                .memory = gpu::Memory::CpuVisible,
            });
        const auto staging_guard = ScopeExit{[&] { gpu::destroy_buffer(m_device.get(), staging_buffer); }};
        std::memcpy(gpu::buffer_host_address(staging_buffer), desc.data.data, desc.data.size);

        auto* command_allocator    = gpu::create_command_allocator(m_device.get(), gpu::Queue::Graphics);
        const auto allocator_guard = ScopeExit{[&] { gpu::destroy_command_allocator(m_device.get(), command_allocator); }};
        auto* command_buffer       = gpu::begin_command_buffer(m_device.get(), command_allocator);

        const auto before_copy = gpu::ImageBarrier{
            .image             = image,
            .dst_stages        = gpu::PipelineStage::Transfer,
            .dst_access        = gpu::Access::TransferWrite,
            .old_layout        = gpu::ImageLayout::Undefined,
            .new_layout        = gpu::ImageLayout::TransferDst,
            .subresource_range = {0u, 1u, 0u, 1u},
        };
        gpu::barrier(command_buffer, {.image_barriers = {&before_copy, 1}});

        const auto copy = gpu::BufferImageCopy{
            .image_subresource = {0u, 0u, 1u},
            .image_extent      = {desc.width, desc.height},
        };
        gpu::copy_buffer_to_image(command_buffer, staging_buffer, image, {&copy, 1});

        const auto after_copy = gpu::ImageBarrier{
            .image             = image,
            .src_stages        = gpu::PipelineStage::Transfer,
            .src_access        = gpu::Access::TransferWrite,
            .dst_stages        = gpu::PipelineStage::FragmentShader,
            .dst_access        = gpu::Access::ShaderRead,
            .old_layout        = gpu::ImageLayout::TransferDst,
            .new_layout        = gpu::ImageLayout::ShaderReadOnly,
            .subresource_range = {0u, 1u, 0u, 1u},
        };
        gpu::barrier(command_buffer, {.image_barriers = {&after_copy, 1}});

        const auto timeline_point = gpu::submit(
            m_device.get(), gpu::Queue::Graphics,
            gpu::Submit{.command_buffers = {{command_buffer}}});
        // Keep the staging buffer and allocator alive until the upload completes.
        gpu::wait_timeline(m_device.get(), timeline_point);

        return texture;
    }

    void Renderer::destroy_texture(Texture texture)
    {
        m_pending_deletions.push([this, texture] {
            m_texture_manager.destroy_texture(texture);
        });
    }

} // namespace orion
