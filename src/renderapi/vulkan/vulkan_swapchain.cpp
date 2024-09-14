#include "vulkan_swapchain.h"

#include "vulkan_error.h"

#include "orion/assertion.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <vector>

namespace orion
{
    VulkanSwapchain::VulkanSwapchain(VkDevice device,
                                     VkInstance instance,
                                     VkPhysicalDevice physical_device,
                                     VkSurfaceKHR surface,
                                     VkSwapchainKHR swapchain,
                                     VkSwapchainCreateInfoKHR swapchain_info,
                                     VulkanQueue* queue,
                                     VulkanContext* context)
        : device_(device)
        , instance_(instance)
        , physical_device_(physical_device)
        , surface_(surface)
        , swapchain_(swapchain)
        , swapchain_info_(swapchain_info)
        , queue_(queue)
        , context_(context)
    {
        // Get images and create image views
        const auto images = get_images();
        create_image_views(images);

        // Create semaphores per image (image available + render finished)
        semaphores_.resize(images.size() * 2);
        std::generate_n(semaphores_.begin(), images.size(), [&]() {
            VkSemaphore semaphore = VK_NULL_HANDLE;
            const auto info = VkSemaphoreCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
            };
            vk_assert(vkCreateSemaphore(device_, &info, nullptr, &semaphore));
            return semaphore;
        });
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        for (auto render_target : render_targets_) {
            context_->remove(render_target);
        }
        for (auto image : images_) {
            context_->remove(image);
        }
        for (auto semaphore : semaphores_) {
            vkDestroySemaphore(device_, semaphore, nullptr);
        }
        if (swapchain_ != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        }
        if (surface_ != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance_, surface_, nullptr);
        }
    }

    RenderTargetHandle VulkanSwapchain::acquire_render_target_api()
    {
        VkSemaphore wait_semaphore = image_available_semaphore(semaphore_index_);
        vk_assert(vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, wait_semaphore, VK_NULL_HANDLE, &image_index_));
        VkSemaphore signal_semaphore = render_finished_semaphore(semaphore_index_);
        queue_->vk_queue_wait(wait_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        queue_->vk_queue_signal(signal_semaphore);
        return render_targets_[image_index_];
    }

    ImageHandle VulkanSwapchain::current_image_api()
    {
        ORION_ASSERT(image_index_ != UINT32_MAX && "Image index is not set. Did you call Swapchain::acquire_render_target()");
        return images_[image_index_];
    }

    void VulkanSwapchain::present_api(bool vsync)
    {
        const auto semaphore_index = semaphore_index_;
        semaphore_index_ = (semaphore_index_ + 1) % semaphores_.size();
        VkSemaphore wait_semaphore = render_finished_semaphore(semaphore_index);

        const auto info = VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &wait_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain_,
            .pImageIndices = &image_index_,
            .pResults = nullptr,
        };
        const auto result = vkQueuePresentKHR(queue_->vk_queue(), &info);

        const auto present_mode = vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
        if (result == VK_ERROR_OUT_OF_DATE_KHR || swapchain_info_.presentMode != present_mode) {
            swapchain_info_.presentMode = present_mode;
            vkDeviceWaitIdle(device_);
            recreate_swapchain_and_resources();
        }
    }

    VkSemaphore VulkanSwapchain::image_available_semaphore(std::uint32_t index) const
    {
        return semaphores_[index];
    }

    VkSemaphore VulkanSwapchain::render_finished_semaphore(std::uint32_t index) const
    {
        return semaphores_[index + 1];
    }

    std::vector<VkImage> VulkanSwapchain::get_images()
    {
        // Get swapchain images
        std::uint32_t image_count;
        vk_assert(vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr));
        std::vector<VkImage> images(image_count);
        vk_assert(vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, images.data()));
        SPDLOG_TRACE("vkGetSwapchainImagesKHR returned {} images", image_count);

        // Insert images into context
        std::ranges::for_each(images, [&](VkImage image) {
            images_.push_back(context_->insert(image));
        });

        return images;
    }

    void VulkanSwapchain::create_image_views(const std::vector<VkImage>& images)
    {
        // Create image views
        std::vector<VkImageView> image_views(images.size());
        std::ranges::transform(images, image_views.begin(), [&](VkImage image) {
            VkImageView image_view = VK_NULL_HANDLE;
            const auto info = VkImageViewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchain_info_.imageFormat,
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
            vk_assert(vkCreateImageView(device_, &info, nullptr, &image_view));
            SPDLOG_TRACE("Created VkImageView {} for swapchain image VkImage {}", fmt::ptr(image_view), fmt::ptr(image));
            return image_view;
        });

        // Insert image views into context
        std::ranges::for_each(image_views, [&](VkImageView image_view) {
            render_targets_.push_back(context_->insert(image_view));
        });
    }

    void VulkanSwapchain::recreate_swapchain_and_resources()
    {
        // Cleanup old resources
        for (auto render_target : render_targets_) {
            context_->remove(render_target);
        }
        render_targets_.clear();
        for (auto image : images_) {
            context_->remove(image);
        }
        images_.clear();

        if (swapchain_ != VK_NULL_HANDLE) {
            swapchain_info_.oldSwapchain = swapchain_;
        }

        // Get surface capabilities
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vk_assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_capabilities));

        // User current transform returned by vulkan
        swapchain_info_.preTransform = surface_capabilities.currentTransform;

        // Set extent based on current create info and surface capability
        swapchain_info_.imageExtent.width = std::min(std::max(swapchain_info_.imageExtent.width, surface_capabilities.minImageExtent.width), surface_capabilities.maxImageExtent.width);
        swapchain_info_.imageExtent.height = std::min(std::max(swapchain_info_.imageExtent.height, surface_capabilities.minImageExtent.height), surface_capabilities.maxImageExtent.height);

        // Create new swapchain
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        vk_assert(vkCreateSwapchainKHR(device_, &swapchain_info_, nullptr, &swapchain));

        // Destroy old swapchain & assign new swapchain
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        swapchain_ = swapchain;

        // Get images from new swapchain and create image views
        const auto images = get_images();
        create_image_views(images);
    }
} // namespace orion
