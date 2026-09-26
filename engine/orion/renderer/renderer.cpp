#include "renderer.hpp"

#include "orion/core/window.hpp"
#include "orion/lib/array.hpp"

namespace orion
{
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
        gpu::destroy_swapchain(m_device, m_swapchain);
        gpu::destroy_device(m_device);
    }

    bool Renderer::begin_frame()
    {
        auto& frame_data = current_frame_data();

        // Wait until the previous submission for this frame has completed
        gpu::wait_timeline(m_device, frame_data.timeline_point);

        return true;
    }

    void Renderer::render()
    {
        auto& frame_data = current_frame_data();

        // Begin recording commands for this frame
        auto* command_buffer = gpu::begin_command_buffer(m_device, frame_data.command_allocator);

        // TODO: Record commands here

        // Submit commands for execution
        frame_data.timeline_point = gpu::submit(
            m_device,
            gpu::Queue::Graphics,
            gpu::Submit{
                .command_buffers = {{command_buffer}},
            });
    }

    void Renderer::end_frame()
    {
        // TODO: Present the swapchain image here

        // Increment the frame counter for the next frame
        ++m_frame_counter;
    }
} // namespace orion
