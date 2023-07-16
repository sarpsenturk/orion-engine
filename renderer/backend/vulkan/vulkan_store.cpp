#include "vulkan_store.h"

#include <utility>

namespace orion::vulkan
{
    VkShaderModule VulkanStore::find_shader(ShaderModuleHandle shader_module_handle) const
    {
        return shader_modules_.at(shader_module_handle).get();
    }

    VulkanSwapchain& VulkanStore::find_swapchain(SwapchainHandle swapchain_handle)
    {
        return swapchains_.at(swapchain_handle);
    }

    VkBuffer VulkanStore::find_buffer(GPUBufferHandle buffer_handle) const
    {
        return buffers_.at(buffer_handle).buffer();
    }

    VmaAllocation VulkanStore::find_allocation(GPUBufferHandle buffer_handle) const
    {
        return buffers_.at(buffer_handle).allocation();
    }

    VkRenderPass VulkanStore::find_render_pass(RenderPassHandle render_pass_handle) const
    {
        return render_passes_.at(render_pass_handle).get();
    }

    VulkanRenderTarget& VulkanStore::find_render_target(RenderTargetHandle render_target_handle)
    {
        return render_targets_.at(render_target_handle);
    }

    VkPipeline VulkanStore::find_pipeline(PipelineHandle pipeline_handle) const
    {
        return pipelines_.at(pipeline_handle).pipeline();
    }

    VkPipelineLayout VulkanStore::find_pipeline_layout(PipelineHandle pipeline_handle) const
    {
        return pipelines_.at(pipeline_handle).pipeline_layout();
    }

    VkCommandPool VulkanStore::find_command_pool(CommandPoolHandle command_pool_handle) const
    {
        return command_pools_.at(command_pool_handle).get();
    }

    VkCommandBuffer VulkanStore::find_command_buffer(CommandBufferHandle command_buffer_handle) const
    {
        return command_buffers_.at(command_buffer_handle).get();
    }

    VkDescriptorPool VulkanStore::find_descriptor_pool(DescriptorPoolHandle descriptor_pool_handle) const
    {
        return descriptor_pools_.at(descriptor_pool_handle).get();
    }

    VkDescriptorSet VulkanStore::find_descriptor_set(DescriptorSetHandle descriptor_set_handle) const
    {
        return descriptor_sets_.at(descriptor_set_handle).get();
    }

    VkSemaphore VulkanStore::find_semaphore(SemaphoreHandle semaphore_handle) const
    {
        return semaphores_.at(semaphore_handle).get();
    }

    VkFence VulkanStore::find_fence(FenceHandle fence_handle) const
    {
        return fences_.at(fence_handle).get();
    }

    void VulkanStore::add(ShaderModuleHandle handle, UniqueVkShaderModule shader_module)
    {
        shader_modules_.insert(std::make_pair(handle, std::move(shader_module)));
    }

    void VulkanStore::add(SwapchainHandle handle, UniqueVkSurfaceKHR surface)
    {
        surfaces_.insert(std::make_pair(handle, std::move(surface)));
    }

    void VulkanStore::add(SwapchainHandle handle, VulkanSwapchain swapchain)
    {
        swapchains_.insert(std::make_pair(handle, std::move(swapchain)));
    }

    void VulkanStore::add(GPUBufferHandle handle, VulkanBuffer buffer)
    {
        buffers_.insert(std::make_pair(handle, std::move(buffer)));
    }

    void VulkanStore::add(RenderPassHandle handle, UniqueVkRenderPass render_pass)
    {
        render_passes_.insert(std::make_pair(handle, std::move(render_pass)));
    }

    void VulkanStore::add(RenderTargetHandle handle, VulkanRenderTarget render_target)
    {
        render_targets_.insert(std::make_pair(handle, std::move(render_target)));
    }

    void VulkanStore::add(PipelineHandle handle, VulkanPipeline pipeline)
    {
        pipelines_.insert(std::make_pair(handle, std::move(pipeline)));
    }

    void VulkanStore::add(CommandPoolHandle handle, UniqueVkCommandPool command_pool)
    {
        command_pools_.insert(std::make_pair(handle, std::move(command_pool)));
    }

    void VulkanStore::add(CommandBufferHandle handle, UniqueVkCommandBuffer command_buffer)
    {
        command_buffers_.insert(std::make_pair(handle, std::move(command_buffer)));
    }

    void VulkanStore::add(DescriptorPoolHandle handle, UniqueVkDescriptorPool descriptor_pool)
    {
        descriptor_pools_.insert(std::make_pair(handle, std::move(descriptor_pool)));
    }

    void VulkanStore::add(DescriptorSetHandle handle, UniqueVkDescriptorSet descriptor_set)
    {
        descriptor_sets_.insert(std::make_pair(handle, std::move(descriptor_set)));
    }

    void VulkanStore::add(SemaphoreHandle handle, UniqueVkSemaphore semaphore)
    {
        semaphores_.insert(std::make_pair(handle, std::move(semaphore)));
    }

    void VulkanStore::add(FenceHandle handle, UniqueVkFence fence)
    {
        fences_.insert(std::make_pair(handle, std::move(fence)));
    }

    void VulkanStore::remove(ShaderModuleHandle shader_module_handle)
    {
        shader_modules_.erase(shader_module_handle);
    }

    void VulkanStore::remove(SwapchainHandle swapchain_handle)
    {
        swapchains_.erase(swapchain_handle);
        surfaces_.erase(swapchain_handle);
    }

    void VulkanStore::remove(GPUBufferHandle buffer_handle)
    {
        buffers_.erase(buffer_handle);
    }

    void VulkanStore::remove(RenderPassHandle render_pass_handle)
    {
        render_passes_.erase(render_pass_handle);
    }

    void VulkanStore::remove(RenderTargetHandle render_target_handle)
    {
        render_targets_.erase(render_target_handle);
    }

    void VulkanStore::remove(PipelineHandle pipeline_handle)
    {
        pipelines_.erase(pipeline_handle);
    }

    void VulkanStore::remove(CommandPoolHandle command_pool_handle)
    {
        command_pools_.erase(command_pool_handle);
    }

    void VulkanStore::remove(CommandBufferHandle command_buffer_handle)
    {
        command_buffers_.erase(command_buffer_handle);
    }

    void VulkanStore::remove(DescriptorPoolHandle descriptor_pool_handle)
    {
        descriptor_pools_.erase(descriptor_pool_handle);
    }

    void VulkanStore::remove(DescriptorSetHandle descriptor_set_handle)
    {
        descriptor_sets_.erase(descriptor_set_handle);
    }

    void VulkanStore::remove(SemaphoreHandle semaphore_handle)
    {
        semaphores_.erase(semaphore_handle);
    }

    void VulkanStore::remove(FenceHandle fence_handle)
    {
        fences_.erase(fence_handle);
    }

    void VulkanStore::add_or_assign(SwapchainHandle handle, VulkanSwapchain swapchain)
    {
        swapchains_.insert_or_assign(handle, std::move(swapchain));
    }

    void VulkanStore::add_or_assign(RenderTargetHandle handle, VulkanRenderTarget render_target)
    {
        render_targets_.insert_or_assign(handle, std::move(render_target));
    }
} // namespace orion::vulkan
