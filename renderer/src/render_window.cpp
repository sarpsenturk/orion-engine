#include "orion-renderer/render_window.h"

#include "orion-renderapi/render_device.h"

#include "orion-core/window.h"

namespace orion
{
    RenderWindow::RenderWindow(std::unique_ptr<Swapchain> swapchain, std::array<RenderTarget, frames_in_flight> render_targets)
        : swapchain_(std::move(swapchain))
        , render_targets_(std::move(render_targets))
    {
    }

    RenderWindow RenderWindow::create(RenderDevice* device, const Window* window)
    {
        const auto swapchain_desc = SwapchainDesc{
            .image_count = frames_in_flight,
            .image_format = Format::B8G8R8A8_Srgb,
            .image_size = window->size(),
            .image_usage = ImageUsageFlags::ColorAttachment,
            .vsync = true, // TODO: make this configurable
        };
        auto swapchain = device->create_swapchain(*window, swapchain_desc);
        auto make_render_target = [device, window, swapchain_ptr = swapchain.get()](frame_index_t frame_index) {
            const auto render_target_desc = RenderTargetDesc{
                .size = window->size(),
                .image_usage = ImageUsageFlags::ColorAttachment,
                .initial_layout = ImageLayout::Undefined,
                .final_layout = ImageLayout::PresentSrc,
            };
            return RenderTarget::create(device, swapchain_ptr->get_image(frame_index), render_target_desc);
        };

        return RenderWindow{std::move(swapchain), generate_per_frame(make_render_target)};
    }

    void RenderWindow::present(std::span<const SemaphoreHandle> wait_semaphores)
    {
        swapchain_->present(wait_semaphores);
    }
} // namespace orion
