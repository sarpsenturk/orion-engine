#pragma once

#include "orion/renderapi/swapchain.h"

#include "vulkan_context.h"
#include "vulkan_queue.h"

#include <Volk/volk.h>

#include <vector>

namespace orion
{
    class VulkanSwapchain final : public Swapchain
    {
    public:
        VulkanSwapchain(
            VkDevice device,
            VkInstance instance,
            VkPhysicalDevice physical_device,
            VkSurfaceKHR surface,
            VkSwapchainKHR swapchain,
            VkSwapchainCreateInfoKHR swapchain_info_,
            VulkanQueue* queue,
            VulkanContext* context);
        ~VulkanSwapchain() override;

    private:
        RenderTargetHandle acquire_render_target_api() override;
        ImageHandle current_image_api() override;
        void present_api(bool vsync) override;

        VkSemaphore image_available_semaphore(std::uint32_t index) const;
        VkSemaphore render_finished_semaphore(std::uint32_t index) const;

        std::vector<VkImage> get_images();
        void create_image_views(const std::vector<VkImage>& images);
        void recreate_swapchain_and_resources();

        VkDevice device_;
        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        VkSurfaceKHR surface_;
        VkSwapchainKHR swapchain_;
        VkSwapchainCreateInfoKHR swapchain_info_;
        VulkanQueue* queue_;
        VulkanContext* context_;

        std::vector<ImageHandle> images_;
        std::vector<RenderTargetHandle> render_targets_;
        // Both image available semaphores & render finished semaphores are stored in this vector
        // semaphores_[semaphore_index_] -> image_available_semaphore
        // semaphores_[semaphore_index_ + 1] -> render_finished_semaphore
        std::vector<VkSemaphore> semaphores_;

        std::uint32_t semaphore_index_ = 0;
        std::uint32_t image_index_ = UINT32_MAX;
    };
} // namespace orion
