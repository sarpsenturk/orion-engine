#pragma once

#include "orion/renderapi/swapchain.h"

#include <Volk/volk.h>

namespace orion
{
    class VulkanSwapchain final : public Swapchain
    {
    public:
        VulkanSwapchain(VkDevice device, VkInstance instance, VkSurfaceKHR surface, VkSwapchainKHR swapchain);
        ~VulkanSwapchain() override;

    private:
        VkDevice device_;
        VkInstance instance_;
        VkSurfaceKHR surface_;
        VkSwapchainKHR swapchain_;
    };
} // namespace orion
