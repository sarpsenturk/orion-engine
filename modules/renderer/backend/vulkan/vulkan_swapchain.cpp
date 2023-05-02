#include "vulkan_swapchain.h"

namespace orion::vulkan
{
    VulkanSwapchain::VulkanSwapchain(UniqueVkSurfaceKHR surface,
                                     UniqueVkSwapchainKHR swapchain,
                                     UniqueVkRenderPass render_pass,
                                     UniqueVkSemaphore image_available_,
                                     std::vector<UniqueVkImageView> image_views,
                                     std::vector<UniqueVkFramebuffer> framebuffers)
        : surface_(std::move(surface))
        , swapchain_(std::move(swapchain))
        , render_pass_(std::move(render_pass))
        , image_available_(std::move(image_available_))
        , image_views_(std::move(image_views))
        , framebuffers_(std::move(framebuffers))
    {
    }

    std::uint32_t VulkanSwapchain::next_image_index()
    {
        VkDevice device = swapchain_.get_deleter().device;
        vk_result_check(vkAcquireNextImageKHR(device, swapchain(), UINT64_MAX, image_semaphore(), VK_NULL_HANDLE, &available_image_));
        return available_image_;
    }
} // namespace orion::vulkan
