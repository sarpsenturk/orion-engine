#pragma once

#include "vulkan_device.h"
#include "vulkan_types.h"

#include "orion-renderapi/swapchain.h"

namespace orion::vulkan
{
    class VulkanSwapchain final : public Swapchain
    {
    public:
        VulkanSwapchain(VulkanDevice* device, UniqueVkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain);

    private:
        VulkanDevice* device_;
        UniqueVkSurfaceKHR surface_;
        UniqueVkSwapchainKHR swapchain_;
    };
} // namespace orion::vulkan
