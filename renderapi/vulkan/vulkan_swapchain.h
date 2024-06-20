#pragma once

#include "orion-renderapi/swapchain.h"

#include "vulkan_types.h"

#include <vector>

namespace orion::vulkan
{
    class VulkanQueue;
    class VulkanResourceManager;

    class VulkanSwapchain final : public Swapchain
    {
    public:
        VulkanSwapchain(VkDevice device, VkPhysicalDevice physical_device, VulkanQueue* queue, VulkanResourceManager* resource_manager, UniqueVkSurfaceKHR surface, VkSwapchainCreateInfoKHR create_info);

    private:
        ImageViewHandle acquire_render_target_api() override;
        ImageHandle current_image_api() override;
        void present_api(std::uint32_t sync_interval) override;

        void release_images();
        void destroy_image_views();
        void recreate_swapchain_and_resources();

        VkDevice device_;
        VkPhysicalDevice physical_device_;
        VulkanQueue* queue_;
        VulkanResourceManager* resource_manager_;
        UniqueVkSurfaceKHR surface_;
        VkSwapchainCreateInfoKHR create_info_;
        UniqueVkSwapchainKHR swapchain_;

        std::vector<ImageHandle> images_;
        std::vector<ImageViewHandle> image_views_;
        std::vector<UniqueVkSemaphore> image_acquired_semaphores_;
        std::vector<UniqueVkSemaphore> render_semaphores_;
        std::uint32_t image_count_ = 0;
        std::uint32_t semaphore_index_ = 0;
        std::uint32_t image_index_ = UINT32_MAX;
    };
} // namespace orion::vulkan
