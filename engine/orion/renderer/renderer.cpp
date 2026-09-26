#include "renderer.hpp"

#include "orion/core/window.hpp"
#include "orion/lib/array.hpp"

#include <fstream>
#include <vector>

namespace orion
{
    static std::vector<uint32_t> load_shader(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + path);
        }

        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint32_t> spirv(size / sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(spirv.data()), size);

        return spirv;
    }

    Renderer::Renderer(const RendererDesc& desc)
        : m_device(gpu::create_device({}))
        , m_swapchain(gpu::create_swapchain(
              m_device,
              gpu::SwapchainDesc{
                  .native_window_handle = desc.window->handle(),
                  .width                = static_cast<std::uint32_t>(desc.window->width()),
                  .height               = static_cast<std::uint32_t>(desc.window->height()),
                  .image_count          = 3u,
                  .image_format         = gpu::Format::BGRA8_Unorm,
              }))
        , m_pso(gpu::create_graphics_pipeline(
              m_device,
              gpu::GraphicsPipelineDesc{
                  .spirv             = load_shader(ORION_SHADER_OUTPUT_DIR "/triangle.spv"),
                  .topology          = gpu::Topology::TriangleList,
                  .cull_mode         = gpu::CullMode::Back,
                  .front_face_ccw    = true,
                  .color_attachments = {{gpu::Format::BGRA8_Unorm}},
              }))
        , m_frame_data(make_array<frames_in_flight>([this]() {
            return FrameData{
                .command_allocator = gpu::create_command_allocator(m_device, gpu::Queue::Graphics),
            };
        }))
    {
    }

    Renderer::~Renderer()
    {
        // Wait until the GPU is idle before tearing down Renderer
        gpu::wait_idle(m_device);

        for (const auto& frame_data : m_frame_data) {
            gpu::destroy_command_allocator(m_device, frame_data.command_allocator);
        }
        gpu::destroy_pipeline_state(m_device, m_pso);
        gpu::destroy_swapchain(m_device, m_swapchain);
        gpu::destroy_device(m_device);
    }

    bool Renderer::begin_frame()
    {
        auto& frame_data = current_frame_data();

        // Wait until the previous submission for this frame has completed
        gpu::wait_timeline(m_device, frame_data.timeline_point);

        // Reset command allocator
        gpu::reset_command_allocator(m_device, frame_data.command_allocator);

        // Acquire the next swapchain frame
        if (auto swapchain_frame = gpu::acquire_swapchain_frame(m_device, m_swapchain)) {
            frame_data.swapchain_frame = *swapchain_frame;
            return true;
        } else {
            return false;
        }
    }

    void Renderer::render()
    {
        auto& frame_data = current_frame_data();

        // Begin recording commands for this frame
        auto* command_buffer = gpu::begin_command_buffer(m_device, frame_data.command_allocator);

        // Transition the swapchain image to a color attachment
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image      = frame_data.swapchain_frame.image,
                        .src_stages = gpu::PipelineStage::ColorAttachmentOutput,
                        .src_access = gpu::Access::None,
                        .dst_stages = gpu::PipelineStage::ColorAttachmentOutput,
                        .dst_access = gpu::Access::ColorAttachmentWrite,
                        .old_layout = gpu::ImageLayout::Undefined,
                        .new_layout = gpu::ImageLayout::ColorAttachment,
                    },
                }},
            });

        // Bind graphics pipeline state (PSO)
        gpu::bind_pso(command_buffer, m_pso);

        // Get swapchain image extent
        const auto swapchain_extent = gpu::get_image_extent_2d(frame_data.swapchain_frame.image);

        // Set viewport and scissor
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
        gpu::set_scissor(command_buffer, gpu::Rect2D{.extent = swapchain_extent});

        // TODO: Record render passes & draw calls

        // Transition the swapchain image to a presentable layout
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image      = frame_data.swapchain_frame.image,
                        .src_stages = gpu::PipelineStage::ColorAttachmentOutput,
                        .src_access = gpu::Access::ColorAttachmentWrite,
                        .dst_stages = gpu::PipelineStage::None,
                        .dst_access = gpu::Access::None,
                        .old_layout = gpu::ImageLayout::ColorAttachment,
                        .new_layout = gpu::ImageLayout::Present,
                    },
                }},
            });

        // Submit commands for execution
        frame_data.timeline_point = gpu::submit(
            m_device,
            gpu::Queue::Graphics,
            gpu::Submit{
                .command_buffers = {{command_buffer}},
                .swapchain       = m_swapchain,
                .swapchain_frame = &frame_data.swapchain_frame,
            });
    }

    void Renderer::end_frame()
    {
        auto& frame_data = current_frame_data();

        // Present the swapchain frame
        gpu::present(m_device, m_swapchain, frame_data.swapchain_frame);

        // Increment the frame counter for the next frame
        ++m_frame_counter;
    }
} // namespace orion
