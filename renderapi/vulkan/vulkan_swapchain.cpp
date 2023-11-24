#include "vulkan_swapchain.h"

#include "vulkan_conversion.h"

namespace orion::vulkan
{
    VulkanSwapchain::VulkanSwapchain(VulkanDevice* device, UniqueVkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain)
        : device_(device)
        , surface_(std::move(surface))
        , swapchain_(std::move(swapchain))
        , image_semaphore_(device->create_vk_semaphore(), SemaphoreDeleter{device->vk_device()})
        , image_fence_(device->create_vk_fence(false), FenceDeleter{device->vk_device()})
    {
    }

    std::uint32_t VulkanSwapchain::current_image_index_api()
    {
        if (image_index_ == request_next_image) {
            VkFence fence = image_fence_.get();
            vk_result_check(vkAcquireNextImageKHR(device_->vk_device(), swapchain_.get(), UINT64_MAX, image_semaphore_.get(), fence, &image_index_));

            // Wait on host until image is available
            // TODO: This may be suboptimal however it's currently the best solution I found
            //  for a uniform interface between D3D12 and Vulkan as I'm not 100% sure
            //  how and when DXGI and D3D12 synchronize accesses to swapchain images from command lists
            vk_result_check(vkWaitForFences(device_->vk_device(), 1, &fence, VK_TRUE, UINT64_MAX));
        }
        return image_index_;
    }

    ImageHandle VulkanSwapchain::get_image_api(std::uint32_t image_index)
    {
        ORION_EXPECTS(image_index < images_.size());
        return images_[image_index_];
    }

    void VulkanSwapchain::resize_images_api(std::uint32_t image_count, Format image_format, const Vector2_u& image_size, ImageUsageFlags image_usage)
    {
        VkSwapchainKHR new_swapchain = device_->create_vk_swapchain({
            .surface = surface_.get(),
            .image_count = image_count,
            .format = to_vulkan_type(image_format),
            .extent = to_vulkan_extent(image_size),
            .usage = to_vulkan_type(image_usage),
            .present_mode = VK_PRESENT_MODE_FIFO_KHR, // TODO: Allow present mode to be changed
            .old_swapchain = swapchain_.get(),
        });

        // Reset swapchain
        swapchain_.reset(new_swapchain);
        // Get new images
        acquire_images();
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
            case VK_SUBOPTIMAL_KHR:
                SPDLOG_LOGGER_WARN(device_->logger(), "Swapchain is suboptimal and should be resized");
                break;
            default:
                vk_result_check(result);
        }

        image_index_ = request_next_image;
    }

    void VulkanSwapchain::acquire_images()
    {
        auto* resource_manager = device_->resource_manager();
        for (auto image : images_) {
            resource_manager->remove(image);
        }
        images_.clear();

        std::uint32_t image_count = 0;
        vk_result_check(vkGetSwapchainImagesKHR(device_->vk_device(), swapchain_.get(), &image_count, nullptr));
        std::vector<VkImage> vk_images(image_count);
        vk_result_check(vkGetSwapchainImagesKHR(device_->vk_device(), swapchain_.get(), &image_count, vk_images.data()));

        images_.reserve(vk_images.size());
        for (VkImage image : vk_images) {
            const auto handle = ImageHandle::generate();
            resource_manager->add(handle, image);
            images_.push_back(handle);
        }
    }
} // namespace orion::vulkan
