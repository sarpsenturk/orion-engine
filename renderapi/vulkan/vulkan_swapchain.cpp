#include "vulkan_swapchain.h"

#include "orion-utils/assertion.h"

#include <algorithm>

namespace orion::vulkan
{
    VulkanSwapchain::VulkanSwapchain(VkDevice device, VkPhysicalDevice physical_device, VulkanQueue* queue, UniqueVkSurfaceKHR surface, VkSwapchainCreateInfoKHR create_info)
        : device_(device)
        , physical_device_(physical_device)
        , queue_(queue)
        , surface_(std::move(surface))
        , create_info_(create_info)
    {
        recreate_swapchain_and_resources();
    }

    void VulkanSwapchain::present_api(std::uint32_t sync_interval)
    {
        ORION_EXPECTS((sync_interval == 0 || sync_interval == 1) && "Vulkan only supports sync intervals of 0 or 1");

        // Vulkan swapchains must be recreated internally if the present mode has been changed
        VkPresentModeKHR present_mode = sync_interval == 0 ? VK_PRESENT_MODE_IMMEDIATE_KHR : VK_PRESENT_MODE_FIFO_KHR;
        if (create_info_.presentMode != present_mode) {
            recreate_swapchain_and_resources();
        }
        create_info_.presentMode = present_mode;

        const auto present_info = VkPresentInfoKHR{};
    }

    void VulkanSwapchain::recreate_swapchain_and_resources()
    {
        // I wait for idle before destroying and recreating swapchain and image views, etc.
        // It definitely isn't fast but, I don't know if this is enough either.
        vkDeviceWaitIdle(device_);

        // Destroy old swapchain resources
        image_views_.clear();
        images_.clear();

        // VkSwapchainCreateInfoKHR takes the old VkSwapchainKHR handle
        // which may help it reuse resources
        if (swapchain_ != VK_NULL_HANDLE) {
            create_info_.oldSwapchain = swapchain_.get();
        }

        // Get surface capabilities
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vk_result_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_.get(), &surface_capabilities));

        // User current transform returned by vulkan
        create_info_.preTransform = surface_capabilities.currentTransform;

        // Set extent based on current create info and surface capability
        create_info_.imageExtent.width = std::min(std::max(create_info_.imageExtent.width, surface_capabilities.minImageExtent.width), surface_capabilities.maxImageExtent.width);
        create_info_.imageExtent.height = std::min(std::max(create_info_.imageExtent.height, surface_capabilities.minImageExtent.height), surface_capabilities.maxImageExtent.height);

        // Create new swapchain
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        vk_result_check(vkCreateSwapchainKHR(device_, &create_info_, alloc_callbacks(), &swapchain));

        // This assignment also destroys the old VkSwapchainKHR handle
        swapchain_ = unique(swapchain, device_);

        // Acquire swapchain owned images
        std::uint32_t image_count;
        vk_result_check(vkGetSwapchainImagesKHR(device_, swapchain, &image_count, nullptr));
        images_.resize(image_count);
        vk_result_check(vkGetSwapchainImagesKHR(device_, swapchain, &image_count, images_.data()));

        // Create image views from acquired images
        image_views_.resize(images_.size());
        std::ranges::transform(images_, image_views_.begin(), [&](VkImage image) {
            VkImageView image_view = VK_NULL_HANDLE;
            const auto image_view_info = VkImageViewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = create_info_.imageFormat,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            vk_result_check(vkCreateImageView(device_, &image_view_info, alloc_callbacks(), &image_view));
            return unique(image_view, device_);
        });
    }
} // namespace orion::vulkan
