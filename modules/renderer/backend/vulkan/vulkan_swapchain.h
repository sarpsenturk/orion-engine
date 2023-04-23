#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include <vector> // std::vector

namespace orion::vulkan
{
    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(UniqueVkSurfaceKHR surface,
                        UniqueVkSwapchainKHR swapchain,
                        UniqueVkRenderPass render_pass,
                        UniqueVkSemaphore image_available,
                        std::vector<UniqueVkImageView> image_views,
                        std::vector<UniqueVkFramebuffer> framebuffers);

        [[nodiscard]] auto surface() const noexcept { return surface_.get(); }
        [[nodiscard]] auto swapchain() const noexcept { return swapchain_.get(); }
        [[nodiscard]] auto render_pass() const noexcept { return render_pass_.get(); }
        [[nodiscard]] auto image_available() const noexcept { return image_available_.get(); }
        [[nodiscard]] auto framebuffer(std::uint32_t index) const noexcept { return framebuffers_[index].get(); }

    private:
        UniqueVkSurfaceKHR surface_;
        UniqueVkSwapchainKHR swapchain_;
        UniqueVkRenderPass render_pass_;
        UniqueVkSemaphore image_available_;
        std::vector<UniqueVkImageView> image_views_;
        std::vector<UniqueVkFramebuffer> framebuffers_;
    };
} // namespace orion::vulkan
