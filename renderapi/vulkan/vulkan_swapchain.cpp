#include "vulkan_swapchain.h"

namespace orion::vulkan
{
    VulkanSwapchain::VulkanSwapchain(VulkanDevice* device, UniqueVkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain)
        : device_(device)
        , surface_(std::move(surface))
        , swapchain_(std::move(swapchain))
        , images_(acquire_swapchain_images())
        , image_semaphore_(device->create_vk_semaphore())
    {
    }

    uint32_t VulkanSwapchain::current_image_index_api()
    {
        if (image_index_ == request_next_image) {
            vk_result_check(vkAcquireNextImageKHR(device_->device(), swapchain_.get(), UINT64_MAX, image_semaphore_.get(), VK_NULL_HANDLE, &image_index_));
        }
        return image_index_;
    }

    void VulkanSwapchain::present_api()
    {
        VkSwapchainKHR swapchain = swapchain_.get();
        VkSemaphore image_semaphore = image_semaphore_.get();
        const auto image_index = current_image_index_api();
        const auto info = VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &image_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &image_index,
        };

        switch (auto result = vkQueuePresentKHR(device_->graphics_queue(), &info)) {
            case VK_SUCCESS:
                break;
            case VK_SUBOPTIMAL_KHR:
                SPDLOG_LOGGER_WARN(device_->logger(), "Swapchain is suboptimal");
                break;
            default:
                vk_result_check(result);
        }

        image_index_ = request_next_image;
    }

    std::vector<VkImage> VulkanSwapchain::acquire_swapchain_images()
    {
        std::uint32_t image_count = 0;
        vk_result_check(vkGetSwapchainImagesKHR(device_->device(), swapchain_.get(), &image_count, nullptr));
        std::vector<VkImage> images(image_count);
        vk_result_check(vkGetSwapchainImagesKHR(device_->device(), swapchain_.get(), &image_count, images.data()));
        return images;
    }
} // namespace orion::vulkan
