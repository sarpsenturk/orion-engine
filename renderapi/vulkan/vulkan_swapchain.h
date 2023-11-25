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

        std::uint32_t current_image_index_api() override;
        ImageHandle get_image_api(std::uint32_t image_index) override;
        void resize_images_api(const SwapchainDesc& desc) override;
        void present_api() override;

        void acquire_images();

        VulkanDevice* device_;
        UniqueVkSurfaceKHR surface_;
        UniqueVkSwapchainKHR swapchain_;
        UniqueVkSemaphore image_semaphore_;
        UniqueVkFence image_fence_;

        std::uint32_t image_index_ = request_next_image;

        std::vector<ImageHandle> images_;
    };
} // namespace orion::vulkan
