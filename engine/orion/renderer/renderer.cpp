#include "renderer.hpp"

#include "orion/core/assert.hpp"
#include "orion/core/logger.hpp"
#include "orion/core/window.hpp"
#include "orion/utils/array.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <vector>

namespace orion
{
    static std::vector<std::uint32_t> load_spirv(const char* path)
    {
        // Load spirv from disk
        auto file = std::ifstream(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            ORION_LOG_ERROR(LogChannel::Renderer, "Failed to open SPIR-V file: {}", path);
            throw std::runtime_error(fmt::format("Failed to open SPIR-V file: {}", path));
        }
        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<std::uint32_t> spirv(size / sizeof(std::uint32_t));
        file.read(reinterpret_cast<char*>(spirv.data()), size);
        return spirv;
    }

    Renderer::Renderer()
        : m_device(gpu::create_device(gpu::DeviceDesc{}), gpu::destroy_device)
        , m_frame_data(make_array<frames_in_flight>([this] {
            return FrameInFlight{
                .command_allocator = gpu::create_command_allocator(m_device.get(), gpu::Queue::Graphics),
            };
        }))
        , m_dummy_pipeline(gpu::create_graphics_pipeline(
              m_device.get(),
              gpu::GraphicsPipelineDesc{
                  .spirv             = load_spirv("../shaders/triangle.spv"),
                  .topology          = gpu::Topology::TriangleList,
                  .rasterizer        = gpu::RasterizerDesc{},
                  .depth_stencil     = gpu::DepthStencilDesc{},
                  .color_attachments = {{gpu::Format::BGRA8_Unorm}},
                  .depth_attachment  = gpu::Format::Undefined,
              }))
    {
    }

    Renderer::~Renderer()
    {
        // Wait until all work on the GPU has finished before tearing down the Renderer
        gpu::wait_idle(m_device.get());

        gpu::destroy_pipeline(m_device.get(), m_dummy_pipeline);

        for (auto& frame : m_frame_data) {
            gpu::destroy_command_allocator(m_device.get(), frame.command_allocator);
        }

        for (auto [_, swapchain] : m_swapchains) {
            gpu::destroy_swapchain(m_device.get(), swapchain);
        }
    }

    std::optional<Frame> Renderer::begin_frame(gpu::Swapchain* swapchain)
    {
        auto& frame_data = current_frame();
        // Wait until the previous submission of this frame has finished
        gpu::wait_timeline(m_device.get(), frame_data.timeline_point);
        // Reset the command allocator for this frame
        gpu::reset_command_allocator(m_device.get(), frame_data.command_allocator);
        // Acquire the next frame from the swapchain
        // Return a Frame object if successful, otherwise return std::nullopt
        if (auto swapchain_frame = gpu::acquire_swapchain_frame(m_device.get(), swapchain)) {
            frame_data.swapchain_frame = *swapchain_frame;
            return Frame{};
        } else {
            ORION_LOG_WARN(LogChannel::Renderer, "Swapchain frame acquisition failed, possibly due to window resize or minimization");
            return std::nullopt;
        }
    }

    void Renderer::end_frame()
    {
        auto& frame_data = current_frame();
        // Begin command buffer recording
        auto* command_buffer = gpu::begin_command_buffer(m_device.get(), frame_data.command_allocator);
        // Transition the swapchain image to color attachment optimal layout
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image             = frame_data.swapchain_frame.image,
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
                }},
            });
        // Begin render pass
        gpu::begin_render_pass(
            command_buffer,
            gpu::RenderPass{
                .color_attachments = {{
                    gpu::RenderPassAttachment{
                        .image_view = frame_data.swapchain_frame.image_view,
                        .load_op    = gpu::LoadOp::Clear,
                        .store_op   = gpu::StoreOp::Store,
                    },
                }},
                .depth_attachment  = std::nullopt,
            });
        // Bind the dummy graphics pipeline
        gpu::bind_pipeline_state(command_buffer, m_dummy_pipeline);
        // Get the extent of the swapchain image
        const auto swapchain_extent = gpu::image_extent_2d(frame_data.swapchain_frame.image);
        // Set viewport and scissor to cover the entire swapchain image
        gpu::set_viewport(
            command_buffer,
            gpu::Viewport{
                .x         = 0.0f,
                .y         = 0.0f,
                .width     = static_cast<float>(swapchain_extent.width),
                .height    = static_cast<float>(swapchain_extent.height),
                .min_depth = 0.0f,
                .max_depth = 1.0f,
            });
        gpu::set_scissor(
            command_buffer,
            gpu::Rect2D{
                .offset = {0, 0},
                .extent = swapchain_extent,
            });
        // Issue a draw call for a triangle
        gpu::draw(command_buffer, {3u, 1u, 0u, 0u});
        // End render pass
        gpu::end_render_pass(command_buffer);
        // Transition the swapchain image to present source layout
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image             = frame_data.swapchain_frame.image,
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
        frame_data.timeline_point = gpu::submit(
            m_device.get(),
            gpu::Queue::Graphics,
            gpu::Submit{
                .command_buffers = {{command_buffer}},
                .swapchain_frame = &frame_data.swapchain_frame,
            });
        // Present the swapchain image
        gpu::present(m_device.get(), frame_data.swapchain_frame);
        // Increment the frame counter
        ++m_frame_counter;
    }

    gpu::Swapchain* Renderer::create_swapchain(
        const Window& window,
        std::uint32_t image_count,
        gpu::Format image_format)
    {
        ORION_ASSERT(!m_swapchains.contains(&window) && "A swapchain for this window already exists");

        auto* swapchain = gpu::create_swapchain(
            m_device.get(),
            gpu::SwapchainDesc{
                .native_window_handle = window.handle(),
                .width                = static_cast<std::uint32_t>(window.width()),
                .height               = static_cast<std::uint32_t>(window.height()),
                .image_count          = image_count,
                .image_format         = image_format,
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
} // namespace orion
