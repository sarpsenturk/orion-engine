#pragma once

#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_device.h"
#include "vulkan_headers.h"
#include "vulkan_pipeline.h"
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

        VkShaderModule find_shader(ShaderModuleHandle shader_module_handle);
        const VulkanSwapchain& find_swapchain(SwapchainHandle swapchain_handle);

    private:
        SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc, SwapchainHandle existing) override;
        ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc, ShaderModuleHandle existing) override;
        GraphicsPipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc, GraphicsPipelineHandle existing) override;

        void destroy(SwapchainHandle swapchain_handle) override;
        void destroy(ShaderModuleHandle shader_module_handle) override;
        void destroy(GraphicsPipelineHandle graphics_pipeline_handle) override;

        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        UniqueVkDevice device_;
        VulkanQueues queues_;

        std::unordered_map<SwapchainHandle, VulkanSwapchain> swapchains_;
        std::unordered_map<ShaderModuleHandle, UniqueVkShaderModule> shader_modules_;
        std::unordered_map<GraphicsPipelineHandle, VulkanPipeline> graphics_pipelines_;
    };
} // namespace orion::vulkan
