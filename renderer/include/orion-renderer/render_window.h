#pragma once

#include "orion-renderer/config.h"
#include "orion-renderer/render_target.h"

#include "orion-renderapi/swapchain.h"

#include "orion-utils/static_vector.h"

namespace orion
{
    class RenderWindow
    {
    public:
        RenderWindow(std::unique_ptr<Swapchain> swapchain, static_vector<RenderTarget, frames_in_flight> render_targets);

        void present(std::span<const SemaphoreHandle> wait_semaphores);
        [[nodiscard]] auto& acquire_render_target() const { return render_targets_[swapchain_->current_image_index()]; }

    private:
        std::unique_ptr<Swapchain> swapchain_;
        static_vector<RenderTarget, frames_in_flight> render_targets_;
    };

    class RenderDevice;
    class Window;
    RenderWindow create_render_window(RenderDevice* device, const Window* window);
} // namespace orion
