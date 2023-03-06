#pragma once

#include "vulkan_headers.h"

namespace orion::vulkan
{
    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(UniqueVkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain);

        [[nodiscard]] auto surface() const noexcept { return surface_.get(); }
        [[nodiscard]] auto swapchain() const noexcept { return swapchain_.get(); }

    private:
        UniqueVkSurfaceKHR surface_;
        UniqueVkSwapchainKHR swapchain_;
    };
} // namespace orion::vulkan
