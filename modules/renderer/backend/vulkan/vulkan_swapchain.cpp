#include "vulkan_swapchain.h"

namespace orion::vulkan
{
    VulkanSwapchain::VulkanSwapchain(UniqueVkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain)
        : surface_(std::move(surface))
        , swapchain_(std::move(swapchain))
    {
    }
} // namespace orion::vulkan
