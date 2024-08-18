#include "vulkan_swapchain.h"

namespace orion
{
    VulkanSwapchain::VulkanSwapchain(VkDevice device, VkInstance instance, VkSurfaceKHR surface, VkSwapchainKHR swapchain)
        : device_(device)
        , instance_(instance)
        , surface_(surface)
        , swapchain_(swapchain)
    {
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        if (swapchain_ != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        }
        if (surface_ != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance_, surface_, nullptr);
        }
    }
} // namespace orion
