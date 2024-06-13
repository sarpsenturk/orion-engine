#pragma once

#include "orion-renderapi/swapchain.h"

#include "vulkan_types.h"

#include <vector>

namespace orion::vulkan
{
    class VulkanQueue;

    class VulkanSwapchain final : public Swapchain
    {
    public:
        VulkanSwapchain(VkDevice device, VkPhysicalDevice physical_device, VulkanQueue* queue, UniqueVkSurfaceKHR surface, VkSwapchainCreateInfoKHR create_info);

    private:
        void present_api(std::uint32_t sync_interval) override;

        void recreate_swapchain_and_resources();

        VkDevice device_;
        VkPhysicalDevice physical_device_;
        VulkanQueue* queue_;
        UniqueVkSurfaceKHR surface_;
        VkSwapchainCreateInfoKHR create_info_;
        UniqueVkSwapchainKHR swapchain_;

        std::vector<VkImage> images_;
        std::vector<UniqueVkImageView> image_views_;
        std::vector<UniqueVkSemaphore> image_acquired_semaphores_;
    };
} // namespace orion::vulkan
