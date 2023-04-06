#pragma once

#include "orion-renderapi/handles.h"
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
        RenderPassHandle create_render_pass_api(const RenderPassDesc& desc, RenderPassHandle existing) override;

        void destroy(SwapchainHandle swapchain_handle) override;
        void destroy(RenderPassHandle render_pass_handle) override;

        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        UniqueVkDevice device_;
        VulkanQueues queues_;

        std::unordered_map<SwapchainHandle, VulkanSwapchain> swapchains_;
        std::unordered_map<RenderPassHandle, UniqueVkRenderPass> render_passes_;
    };
} // namespace orion::vulkan
