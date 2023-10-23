#include "vulkan_swapchain.h"

namespace orion::vulkan
{
    VulkanSwapchain::VulkanSwapchain(VulkanDevice* device, UniqueVkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain)
        : device_(device)
        , surface_(std::move(surface))
        , swapchain_(std::move(swapchain))
    {
    }
} // namespace orion::vulkan
