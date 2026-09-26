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
        for (const auto& frame_data : m_frame_data) {
            gpu::destroy_command_allocator(m_device, frame_data.command_allocator);
        }
        gpu::destroy_swapchain(m_device, m_swapchain);
        gpu::destroy_device(m_device);
    }
} // namespace orion
