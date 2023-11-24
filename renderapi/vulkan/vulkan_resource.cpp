#include "vulkan_resource.h"

#include "orion-utils/assertion.h"

namespace orion::vulkan
{
    VulkanResourceManager::VulkanResourceManager(VkDevice device, VmaAllocator vma_allocator)
        : device_(device)
        , vma_allocator_(vma_allocator)
    {
        ORION_ENSURES(device_ != VK_NULL_HANDLE);
        ORION_ENSURES(vma_allocator_ != VK_NULL_HANDLE);
    }

    void VulkanResourceManager::add(ImageHandle handle, VkImage image, VmaAllocation allocation)
    {
        images_.insert(std::make_pair(handle, unique(image, vma_allocator_, allocation)));
    }

    void VulkanResourceManager::add(ImageHandle handle, VkImage image)
    {
        images_.insert(std::make_pair(handle, unique(image, VK_NULL_HANDLE, VK_NULL_HANDLE)));
    }

    void VulkanResourceManager::add(ImageViewHandle handle, VkImageView image_view)
    {
        image_views_.insert(std::make_pair(handle, unique(image_view, device_)));
    }

    void VulkanResourceManager::add(RenderPassHandle handle, VkRenderPass render_pass)
    {
        render_passes_.insert(std::make_pair(handle, unique(render_pass, device_)));
    }

    void VulkanResourceManager::add(FramebufferHandle handle, VkFramebuffer framebuffer)
    {
        framebuffers_.insert(std::make_pair(handle, unique(framebuffer, device_)));
    }

    void VulkanResourceManager::add(ShaderModuleHandle handle, VkShaderModule shader_module)
    {
        shader_modules_.insert(std::make_pair(handle, unique(shader_module, device_)));
    }

    void VulkanResourceManager::add(DescriptorLayoutHandle handle, VkDescriptorSetLayout descriptor_set_layout)
    {
        descriptor_set_layouts_.insert(std::make_pair(handle, unique(descriptor_set_layout, device_)));
    }

    void VulkanResourceManager::add(DescriptorHandle handle, VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool)
    {
        descriptor_sets_.insert(std::make_pair(handle, unique(descriptor_set, device_, descriptor_pool)));
    }

    void VulkanResourceManager::add(PipelineLayoutHandle handle, VkPipelineLayout pipeline_layout)
    {
        pipeline_layouts_.insert(std::make_pair(handle, unique(pipeline_layout, device_)));
    }

    void VulkanResourceManager::add(PipelineHandle handle, VkPipeline pipeline)
    {
        pipelines_.insert(std::make_pair(handle, unique(pipeline, device_)));
    }

    void VulkanResourceManager::add(GPUBufferHandle handle, VkBuffer buffer, VmaAllocation allocation)
    {
        buffers_.insert(std::make_pair(handle, unique(buffer, vma_allocator_, allocation)));
    }

    void VulkanResourceManager::add(SamplerHandle handle, VkSampler sampler)
    {
        samplers_.insert(std::make_pair(handle, unique(sampler, device_)));
    }

    void VulkanResourceManager::add(FenceHandle handle, VkFence fence)
    {
        fences_.insert(std::make_pair(handle, unique(fence, device_)));
    }

    void VulkanResourceManager::add(SemaphoreHandle handle, VkSemaphore semaphore)
    {
        semaphores_.insert(std::make_pair(handle, unique(semaphore, device_)));
    }

    void VulkanResourceManager::remove(ImageHandle handle)
    {
        images_.erase(handle);
    }

    void VulkanResourceManager::remove(ImageViewHandle handle)
    {
        image_views_.erase(handle);
    }

    void VulkanResourceManager::remove(RenderPassHandle handle)
    {
        render_passes_.erase(handle);
    }

    void VulkanResourceManager::remove(FramebufferHandle handle)
    {
        framebuffers_.erase(handle);
    }

    void VulkanResourceManager::remove(ShaderModuleHandle handle)
    {
        shader_modules_.erase(handle);
    }

    void VulkanResourceManager::remove(DescriptorLayoutHandle handle)
    {
        descriptor_set_layouts_.erase(handle);
    }

    void VulkanResourceManager::remove(DescriptorHandle handle)
    {
        descriptor_sets_.erase(handle);
    }

    void VulkanResourceManager::remove(PipelineLayoutHandle handle)
    {
        pipeline_layouts_.erase(handle);
    }

    void VulkanResourceManager::remove(PipelineHandle handle)
    {
        pipelines_.erase(handle);
    }

    void VulkanResourceManager::remove(GPUBufferHandle handle)
    {
        buffers_.erase(handle);
    }

    void VulkanResourceManager::remove(SamplerHandle handle)
    {
        samplers_.erase(handle);
    }

    void VulkanResourceManager::remove(FenceHandle handle)
    {
        fences_.erase(handle);
    }

    void VulkanResourceManager::remove(SemaphoreHandle handle)
    {
        semaphores_.erase(handle);
    }

    VulkanImageResource VulkanResourceManager::find(ImageHandle handle) const noexcept
    {
        if (auto iter = images_.find(handle); iter != images_.end()) {
            return {.image = iter->second.get(), .allocation = iter->second.get_deleter().allocation};
        }
        return {.image = VK_NULL_HANDLE, .allocation = VK_NULL_HANDLE};
    }

    VkImageView VulkanResourceManager::find(ImageViewHandle handle) const noexcept
    {
        if (auto iter = image_views_.find(handle); iter != image_views_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VkRenderPass VulkanResourceManager::find(RenderPassHandle handle) const noexcept
    {
        if (auto iter = render_passes_.find(handle); iter != render_passes_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VkFramebuffer VulkanResourceManager::find(FramebufferHandle handle) const noexcept
    {
        if (auto iter = framebuffers_.find(handle); iter != framebuffers_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VkShaderModule VulkanResourceManager::find(ShaderModuleHandle handle) const noexcept
    {
        if (auto iter = shader_modules_.find(handle); iter != shader_modules_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VkDescriptorSetLayout VulkanResourceManager::find(DescriptorLayoutHandle handle) const noexcept
    {
        if (auto iter = descriptor_set_layouts_.find(handle); iter != descriptor_set_layouts_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VkDescriptorSet VulkanResourceManager::find(DescriptorHandle handle) const noexcept
    {
        if (auto iter = descriptor_sets_.find(handle); iter != descriptor_sets_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VkPipelineLayout VulkanResourceManager::find(PipelineLayoutHandle handle) const noexcept
    {
        if (auto iter = pipeline_layouts_.find(handle); iter != pipeline_layouts_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VkPipeline VulkanResourceManager::find(PipelineHandle handle) const noexcept
    {
        if (auto iter = pipelines_.find(handle); iter != pipelines_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VulkanBufferResource VulkanResourceManager::find(GPUBufferHandle handle) const noexcept
    {
        if (auto iter = buffers_.find(handle); iter != buffers_.end()) {
            return {.buffer = iter->second.get(), .allocation = iter->second.get_deleter().allocation};
        }
        return {.buffer = VK_NULL_HANDLE, .allocation = VK_NULL_HANDLE};
    }

    VkSampler VulkanResourceManager::find(SamplerHandle handle) const noexcept
    {
        if (auto iter = samplers_.find(handle); iter != samplers_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VkFence VulkanResourceManager::find(FenceHandle handle) const noexcept
    {
        if (auto iter = fences_.find(handle); iter != fences_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }

    VkSemaphore VulkanResourceManager::find(SemaphoreHandle handle) const noexcept
    {
        if (auto iter = semaphores_.find(handle); iter != semaphores_.end()) {
            return iter->second.get();
        }
        return VK_NULL_HANDLE;
    }
} // namespace orion::vulkan
