#include "renderer.hpp"

#include "orion/core/assert.hpp"
#include "orion/core/logger.hpp"
#include "orion/core/window.hpp"
#include "orion/utils/array.hpp"

#include <algorithm>

namespace orion
{
    Renderer::Renderer()
        : m_device(gpu::create_device(gpu::DeviceDesc{}), gpu::destroy_device)
        , m_frame_data(make_array<frames_in_flight>([this] {
            return FrameInFlight{
                .command_allocator = gpu::create_command_allocator(m_device.get(), gpu::Queue::Graphics),
            };
        }))
    {
    }

    Renderer::~Renderer()
    {
        for (auto& frame : m_frame_data) {
            gpu::destroy_command_allocator(m_device.get(), frame.command_allocator);
        }

        for (auto* swapchain : m_swapchains) {
            gpu::destroy_swapchain(m_device.get(), swapchain);
        }
    }

    gpu::Swapchain* Renderer::create_swapchain(
        const Window& window,
        std::uint32_t image_count,
        gpu::Format image_format)
    {
        auto* swapchain = gpu::create_swapchain(
            m_device.get(),
            gpu::SwapchainDesc{
                .native_window_handle = window.handle(),
                .width                = static_cast<std::uint32_t>(window.width()),
                .height               = static_cast<std::uint32_t>(window.height()),
                .image_count          = image_count,
                .image_format         = image_format,
            });
        m_swapchains.push_back(swapchain);
        return swapchain;
    }

    void Renderer::destroy_swapchain(gpu::Swapchain* swapchain)
    {
        ORION_ASSERT(swapchain != nullptr);
        auto it = std::ranges::find(m_swapchains, swapchain);
        if (it == m_swapchains.end()) {
            ORION_LOG_WARN(LogChannel::Renderer, "Destroying swapchain {} that was not created by this renderer", fmt::ptr(swapchain));
        } else {
            gpu::destroy_swapchain(m_device.get(), swapchain);
            m_swapchains.erase(it);
        }
    }
} // namespace orion
