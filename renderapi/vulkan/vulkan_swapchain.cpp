#include "vulkan_swapchain.h"

#include "orion-utils/assertion.h"

#include "vulkan_queue.h"
#include "vulkan_resource.h"

#include <algorithm>

namespace orion::vulkan
{
    namespace
    {
        std::vector<UniqueVkSemaphore> create_semaphores(VkDevice device, std::uint32_t count)
        {
            std::vector<UniqueVkSemaphore> semaphores(count);
            std::generate_n(semaphores.begin(), count, [device]() {
                const auto info = VkSemaphoreCreateInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = nullptr, .flags = 0};
                VkSemaphore semaphore;
                vk_result_check(vkCreateSemaphore(device, &info, alloc_callbacks(), &semaphore));
                return unique(semaphore, device);
            });
            return semaphores;
        }
    } // namespace

    VulkanSwapchain::VulkanSwapchain(VkDevice device, VkPhysicalDevice physical_device, VulkanQueue* queue, VulkanResourceManager* resource_manager, UniqueVkSurfaceKHR surface, VkSwapchainCreateInfoKHR create_info)
        : device_(device)
        , physical_device_(physical_device)
        , queue_(queue)
        , resource_manager_(resource_manager)
        , surface_(std::move(surface))
        , create_info_(create_info)
    {
        recreate_swapchain_and_resources();
    }

    ImageViewHandle VulkanSwapchain::acquire_render_target_api()
    {
        VkSemaphore image_semaphore = image_acquired_semaphores_[semaphore_index_].get();
        vk_result_check(vkAcquireNextImageKHR(device_, swapchain_.get(), UINT64_MAX, image_semaphore, VK_NULL_HANDLE, &image_index_));
        queue_->vk_queue_wait(image_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        queue_->vk_queue_signal(render_semaphores_[semaphore_index_].get());
        return image_views_[image_index_];
    }

    ImageHandle VulkanSwapchain::current_image_api()
    {
        return images_[image_index_];
    }

    void VulkanSwapchain::present_api(std::uint32_t sync_interval)
    {
        ORION_EXPECTS((sync_interval == 0 || sync_interval == 1) && "Vulkan only supports sync intervals of 0 or 1");

        const VkSemaphore render_complete = render_semaphores_[semaphore_index_].get();
        const VkSwapchainKHR swapchain = swapchain_.get();

        const auto present_info = VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &render_complete,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &image_index_,
            .pResults = nullptr,
        };

        if (VkResult result = vkQueuePresentKHR(queue_->vk_queue(), &present_info); result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain_and_resources();
        } else {
            vk_result_check(result);
        }

        // semaphore_index_ is incremented here and not when acquiring a render target
        semaphore_index_ = (semaphore_index_ + 1) % image_count_;

        // Vulkan swapchains must be recreated internally if the present mode has been changed
        VkPresentModeKHR present_mode = sync_interval == 0 ? VK_PRESENT_MODE_IMMEDIATE_KHR : VK_PRESENT_MODE_FIFO_KHR;
        if (create_info_.presentMode != present_mode) {
            create_info_.presentMode = present_mode;
            recreate_swapchain_and_resources();
        }
    }

    void VulkanSwapchain::release_images()
    {
        for (const auto image : images_) {
            resource_manager_->destroy(image);
        }
        images_.clear();
    }

    void VulkanSwapchain::destroy_image_views()
    {
        for (const auto image_view : image_views_) {
            resource_manager_->destroy(image_view);
        }
        image_views_.clear();
    }

    void VulkanSwapchain::recreate_swapchain_and_resources()
    {
        // Wait for idle before destroying and recreating swapchain and image views, etc.
        // It definitely isn't fast but even worse, I don't know if this is enough either.
        vkDeviceWaitIdle(device_);

        // Destroy old swapchain resources
        destroy_image_views();
        release_images();

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
        vk_result_check(vkGetSwapchainImagesKHR(device_, swapchain, &image_count_, nullptr));
        std::vector<VkImage> vk_images(image_count_);
        vk_result_check(vkGetSwapchainImagesKHR(device_, swapchain, &image_count_, vk_images.data()));

        // Generate ImageHandles for public API
        images_.resize(image_count_);
        std::ranges::transform(vk_images, images_.begin(), [&](VkImage image) {
            const auto handle = ImageHandle::generate();
            resource_manager_->add(handle, image);
            return handle;
        });

        // Create semaphores
        image_acquired_semaphores_ = create_semaphores(device_, image_count_);
        render_semaphores_ = create_semaphores(device_, image_count_);
        semaphore_index_ = 0;

        // Create image views from acquired images
        image_views_.resize(image_count_);
        std::ranges::transform(vk_images, image_views_.begin(), [&](VkImage image) {
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
            const auto handle = ImageViewHandle::generate();
            resource_manager_->add(handle, image_view);
            return handle;
        });
    }
} // namespace orion::vulkan
