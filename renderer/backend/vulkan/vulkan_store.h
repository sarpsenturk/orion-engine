#pragma once

#include "vulkan_buffer.h"
#include "vulkan_headers.h"
#include "vulkan_pipeline.h"
#include "vulkan_types.h"

#include "orion-renderapi/descriptor.h"
#include "orion-renderapi/handles.h"

#include <memory>

namespace orion::vulkan
{
    class VulkanStore
    {
    public:
        [[nodiscard]] VkShaderModule find_shader(ShaderModuleHandle shader_module_handle) const;
        [[nodiscard]] VulkanSwapchain& find_swapchain(SwapchainHandle swapchain_handle);
        [[nodiscard]] VkBuffer find_buffer(GPUBufferHandle buffer_handle) const;
        [[nodiscard]] VmaAllocation find_allocation(GPUBufferHandle buffer_handle) const;
        [[nodiscard]] VkRenderPass find_render_pass(RenderPassHandle render_pass_handle) const;
        [[nodiscard]] VulkanRenderTarget& find_render_target(RenderTargetHandle render_target_handle);
        [[nodiscard]] VkPipeline find_pipeline(PipelineHandle pipeline_handle) const;
        [[nodiscard]] VkPipelineLayout find_pipeline_layout(PipelineHandle pipeline_handle) const;
        [[nodiscard]] VkCommandPool find_command_pool(CommandPoolHandle command_pool_handle) const;
        [[nodiscard]] VkCommandBuffer find_command_buffer(CommandBufferHandle command_buffer_handle) const;
        [[nodiscard]] VkDescriptorPool find_descriptor_pool(DescriptorPoolHandle descriptor_pool_handle) const;
        [[nodiscard]] VkDescriptorSet find_descriptor_set(DescriptorSetHandle descriptor_set_handle) const;
        [[nodiscard]] VkSemaphore find_semaphore(SemaphoreHandle semaphore_handle) const;
        [[nodiscard]] VkFence find_fence(FenceHandle fence_handle) const;

        void add(ShaderModuleHandle handle, UniqueVkShaderModule shader_module);
        void add(SwapchainHandle handle, UniqueVkSurfaceKHR surface);
        void add(SwapchainHandle handle, VulkanSwapchain swapchain);
        void add(GPUBufferHandle handle, VulkanBuffer buffer);
        void add(RenderPassHandle handle, UniqueVkRenderPass render_pass);
        void add(RenderTargetHandle handle, VulkanRenderTarget render_target);
        void add(PipelineHandle handle, VulkanPipeline pipeline);
        void add(CommandPoolHandle handle, UniqueVkCommandPool command_pool);
        void add(CommandBufferHandle handle, UniqueVkCommandBuffer command_buffer);
        void add(DescriptorPoolHandle handle, UniqueVkDescriptorPool descriptor_pool);
        void add(DescriptorSetHandle handle, UniqueVkDescriptorSet descriptor_set);
        void add(SemaphoreHandle handle, UniqueVkSemaphore semaphore);
        void add(FenceHandle handle, UniqueVkFence fence);

        void remove(ShaderModuleHandle shader_module_handle);
        void remove(SwapchainHandle swapchain_handle);
        void remove(GPUBufferHandle buffer_handle);
        void remove(RenderPassHandle render_pass_handle);
        void remove(RenderTargetHandle render_target_handle);
        void remove(PipelineHandle pipeline_handle);
        void remove(CommandPoolHandle command_pool_handle);
        void remove(CommandBufferHandle command_buffer_handle);
        void remove(DescriptorPoolHandle descriptor_pool_handle);
        void remove(DescriptorSetHandle descriptor_set_handle);
        void remove(SemaphoreHandle semaphore_handle);
        void remove(FenceHandle fence_handle);

        void add_or_assign(SwapchainHandle handle, VulkanSwapchain swapchain);
        void add_or_assign(RenderTargetHandle handle, VulkanRenderTarget render_target);

    private:
        std::unordered_map<SwapchainHandle, UniqueVkSurfaceKHR> surfaces_;
        std::unordered_map<SwapchainHandle, VulkanSwapchain> swapchains_;
        std::unordered_map<RenderPassHandle, UniqueVkRenderPass> render_passes_;
        std::unordered_map<RenderTargetHandle, VulkanRenderTarget> render_targets_;
        std::unordered_map<ShaderModuleHandle, UniqueVkShaderModule> shader_modules_;
        std::unordered_map<PipelineHandle, VulkanPipeline> pipelines_;
        std::unordered_map<GPUBufferHandle, VulkanBuffer> buffers_;
        std::unordered_map<CommandPoolHandle, UniqueVkCommandPool> command_pools_;
        std::unordered_map<CommandBufferHandle, UniqueVkCommandBuffer> command_buffers_;
        std::unordered_map<DescriptorPoolHandle, UniqueVkDescriptorPool> descriptor_pools_;
        std::unordered_map<DescriptorSetHandle, UniqueVkDescriptorSet> descriptor_sets_;
        std::unordered_map<SemaphoreHandle, UniqueVkSemaphore> semaphores_;
        std::unordered_map<FenceHandle, UniqueVkFence> fences_;
    };
} // namespace orion::vulkan
