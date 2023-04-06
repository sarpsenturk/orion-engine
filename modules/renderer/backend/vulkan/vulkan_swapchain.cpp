#include "vulkan_swapchain.h"

namespace orion::vulkan
{
    VulkanSwapchain::VulkanSwapchain(UniqueVkSurfaceKHR surface,
                                     UniqueVkSwapchainKHR swapchain,
                                     std::vector<UniqueVkImageView> image_views,
                                     UniqueVkRenderPass render_pass)
        : surface_(std::move(surface))
        , swapchain_(std::move(swapchain))
        , image_views_(std::move(image_views))
        , render_pass_(std::move(render_pass))
    {
    }
} // namespace orion::vulkan
