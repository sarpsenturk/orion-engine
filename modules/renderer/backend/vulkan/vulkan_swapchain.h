#pragma once

#include "vulkan_headers.h"

namespace orion::vulkan
{
    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(UniqueVkSurfaceKHR surface,
                        UniqueVkSwapchainKHR swapchain,
                        std::vector<UniqueVkImageView> image_views,
                        UniqueVkRenderPass render_pass);

        [[nodiscard]] auto surface() const noexcept { return surface_.get(); }
        [[nodiscard]] auto swapchain() const noexcept { return swapchain_.get(); }

    private:
        UniqueVkSurfaceKHR surface_;
        UniqueVkSwapchainKHR swapchain_;
        std::vector<UniqueVkImageView> image_views_;
        UniqueVkRenderPass render_pass_;
    };
} // namespace orion::vulkan
