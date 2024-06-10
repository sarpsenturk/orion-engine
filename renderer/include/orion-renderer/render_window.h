#pragma once

#include "orion-renderer/frame.h"
#include "orion-renderer/render_target.h"

#include "orion-renderapi/swapchain.h"

namespace orion
{
    class RenderWindow
    {
    public:
        RenderWindow(std::unique_ptr<Swapchain> swapchain, std::array<RenderTarget, frames_in_flight> render_targets);

        void present(std::span<const SemaphoreHandle> wait_semaphores);
        [[nodiscard]] auto& acquire_render_target() const { return render_targets_[swapchain_->current_image_index()]; }

    private:
        std::unique_ptr<Swapchain> swapchain_;
        std::array<RenderTarget, frames_in_flight> render_targets_;
    };

    class RenderDevice;
    class Window;
    RenderWindow create_render_window(RenderDevice* device, const Window* window);
} // namespace orion
