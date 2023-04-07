#include "vulkan_swapchain.h"

namespace orion::vulkan
{
    VulkanSwapchain::VulkanSwapchain(UniqueVkSurfaceKHR surface,
                                     UniqueVkSwapchainKHR swapchain,
                                     UniqueVkRenderPass render_pass,
                                     std::vector<UniqueVkImageView> image_views,
                                     std::vector<UniqueVkFramebuffer> framebuffers)
        : surface_(std::move(surface))
        , swapchain_(std::move(swapchain))
        , render_pass_(std::move(render_pass))
        , image_views_(std::move(image_views))
        , framebuffers_(std::move(framebuffers))
    {
    }
} // namespace orion::vulkan
