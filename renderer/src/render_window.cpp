#include "orion-renderer/render_window.h"

#include "orion-renderapi/render_device.h"

#include "orion-core/window.h"

namespace orion
{
    RenderWindow::RenderWindow(std::unique_ptr<Swapchain> swapchain, static_vector<RenderTarget, frames_in_flight> render_targets)
        : swapchain_(std::move(swapchain))
        , render_targets_(std::move(render_targets))
    {
    }

    void RenderWindow::present(std::span<const SemaphoreHandle> wait_semaphores)
    {
        swapchain_->present(wait_semaphores);
    }

    RenderWindow create_render_window(RenderDevice* device, const Window* window)
    {
        const auto swapchain_desc = SwapchainDesc{
            .image_count = frames_in_flight,
            .image_format = Format::B8G8R8A8_Srgb,
            .image_size = window->size(),
            .image_usage = ImageUsageFlags::ColorAttachment,
            .vsync = true, // TODO: make this configurable
        };
        auto swapchain = device->create_swapchain(*window, swapchain_desc);

        static_vector<RenderTarget, frames_in_flight> render_targets;
        for (int i = 0; i < frames_in_flight; ++i) {
            const auto render_target_desc = RenderTargetDesc{
                .size = window->size(),
                .image_usage = ImageUsageFlags::ColorAttachment,
                .initial_layout = ImageLayout::Undefined,
                .final_layout = ImageLayout::PresentSrc,
            };
            render_targets.push_back(create_render_target(device, swapchain->get_image(i), render_target_desc));
        }

        return RenderWindow{std::move(swapchain), std::move(render_targets)};
    }
} // namespace orion
