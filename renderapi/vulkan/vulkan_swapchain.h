#pragma once

#include "vulkan_device.h"
#include "vulkan_types.h"

#include "orion-renderapi/swapchain.h"

namespace orion::vulkan
{
    class VulkanSwapchain final : public Swapchain
    {
    public:
        VulkanSwapchain(VulkanDevice* device, UniqueVkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain);

    private:
        static constexpr auto request_next_image = UINT32_MAX;

        uint32_t current_image_index_api() override;
        void present_api() override;

        std::vector<VkImage> acquire_swapchain_images();

        VulkanDevice* device_;
        UniqueVkSurfaceKHR surface_;
        UniqueVkSwapchainKHR swapchain_;
        std::vector<VkImage> images_;
        std::uint32_t image_index_ = request_next_image;
        UniqueVkSemaphore image_semaphore_;
    };
} // namespace orion::vulkan
