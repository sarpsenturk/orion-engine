#pragma once

#include "orion-renderapi/render_device.h"
#include "vulkan_headers.h"
#include "vulkan_swapchain.h"
#include "vulkan_types.h"

#include <unordered_map> // std::unordered_map

namespace orion::vulkan
{
    class VulkanDevice : public RenderDevice
    {
    public:
        VulkanDevice(VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues);

        std::unique_ptr<RenderContext> create_render_context() override;

    private:
        SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc, SwapchainHandle existing) override;

        void destroy(SwapchainHandle swapchain_handle) override;

        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        UniqueVkDevice device_;
        VulkanQueues queues_;

        std::unordered_map<SwapchainHandle, VulkanSwapchain> swapchains_;
    };
} // namespace orion::vulkan
