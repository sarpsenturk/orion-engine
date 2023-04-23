#pragma once

#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_device.h"
#include "vulkan_buffer.h"
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
        VulkanDevice(spdlog::logger* logger, VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues);

        std::unique_ptr<RenderContext> create_render_context() override;

        VkShaderModule find_shader(ShaderModuleHandle shader_module_handle) const;
        const VulkanSwapchain& find_swapchain(SwapchainHandle swapchain_handle) const;
        const VulkanBuffer& find_buffer(GPUBufferHandle buffer_handle) const;
        VkRenderPass find_render_pass(RenderTargetHandle render_target_handle) const;
        VkPipeline find_pipeline(PipelineHandle pipeline_handle) const;
        VkCommandBuffer find_command_buffer(CommandBufferHandle command_buffer_handle) const;

        [[nodiscard]] auto device() const noexcept { return device_.get(); }
        [[nodiscard]] auto allocator() const noexcept { return allocator_.get(); }

    private:
        [[nodiscard]] UniqueVmaAllocator create_allocator();
        [[nodiscard]] inline bool transfer_requires_concurrent(std::uint32_t family_index) const noexcept { return queues_.transfer.index != family_index; }

        [[nodiscard]] VkCommandPool graphics_command_pool() const;
        [[nodiscard]] VkCommandPool transfer_command_pool() const;
        [[nodiscard]] VkCommandPool get_command_pool(CommandQueueType queue_type) const;

        SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc, SwapchainHandle existing) override;
        ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc, ShaderModuleHandle existing) override;
        PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc, PipelineHandle existing) override;
        GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc, GPUBufferHandle existing) override;
        CommandBufferHandle create_command_buffer_api(const CommandBufferDesc& desc, CommandBufferHandle existing) override;

        void destroy_api(SwapchainHandle swapchain_handle) override;
        void destroy_api(ShaderModuleHandle shader_module_handle) override;
        void destroy_api(PipelineHandle graphics_pipeline_handle) override;
        void destroy_api(GPUBufferHandle buffer_handle) override;
        void destroy_api(CommandBufferHandle command_buffer_handle) override;

        void* map_api(GPUBufferHandle buffer_handle) override;
        void unmap_api(GPUBufferHandle buffer_handle) override;

        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        UniqueVkDevice device_;
        UniqueVmaAllocator allocator_ = VK_NULL_HANDLE;
        VulkanQueues queues_;

        std::unordered_map<SwapchainHandle, VulkanSwapchain> swapchains_;
        std::unordered_map<ShaderModuleHandle, UniqueVkShaderModule> shader_modules_;
        std::unordered_map<PipelineHandle, VulkanPipeline> pipelines_;
        std::unordered_map<GPUBufferHandle, VulkanBuffer> buffers_;
        std::unordered_map<std::uint32_t, UniqueVkCommandPool> command_pools_;
        std::unordered_map<CommandBufferHandle, UniqueVkCommandBuffer> command_buffers_;
    };
} // namespace orion::vulkan
