#include "vulkan_resource.h"

#include "orion-utils/assertion.h"

#include <algorithm>

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

    void VulkanResourceManager::destroy(ImageHandle handle)
    {
        images_.erase(handle);
    }

    void VulkanResourceManager::destroy(ImageViewHandle handle)
    {
        image_views_.erase(handle);
    }

    void VulkanResourceManager::destroy(RenderPassHandle handle)
    {
        render_passes_.erase(handle);
    }

    void VulkanResourceManager::destroy(FramebufferHandle handle)
    {
        framebuffers_.erase(handle);
    }

    void VulkanResourceManager::destroy(ShaderModuleHandle handle)
    {
        shader_modules_.erase(handle);
    }

    void VulkanResourceManager::destroy(DescriptorLayoutHandle handle)
    {
        descriptor_set_layouts_.erase(handle);
    }

    void VulkanResourceManager::destroy(DescriptorHandle handle)
    {
        descriptor_sets_.erase(handle);
    }

    void VulkanResourceManager::destroy(PipelineLayoutHandle handle)
    {
        pipeline_layouts_.erase(handle);
    }

    void VulkanResourceManager::destroy(PipelineHandle handle)
    {
        pipelines_.erase(handle);
    }

    void VulkanResourceManager::destroy(GPUBufferHandle handle)
    {
        buffers_.erase(handle);
    }

    void VulkanResourceManager::destroy(SamplerHandle handle)
    {
        samplers_.erase(handle);
    }

    void VulkanResourceManager::destroy(FenceHandle handle)
    {
        fences_.erase(handle);
    }

    void VulkanResourceManager::destroy(SemaphoreHandle handle)
    {
        semaphores_.erase(handle);
    }

    void VulkanResourceManager::destroy_flush()
    {
        for (auto [handle, deleter] : deletion_queue_) {
            deleter(this, handle);
        }
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

    std::vector<VulkanImageResource> VulkanResourceManager::find(std::span<const ImageHandle> handles) const
    {
        std::vector<VulkanImageResource> images(handles.size());
        std::ranges::transform(handles, images.begin(), [this](auto handle) { return find(handle); });
        return images;
    }

    std::vector<VkImageView> VulkanResourceManager::find(std::span<const ImageViewHandle> handles) const
    {
        std::vector<VkImageView> image_views(handles.size());
        std::ranges::transform(handles, image_views.begin(), [this](auto handle) { return find(handle); });
        return image_views;
    }

    std::vector<VkRenderPass> VulkanResourceManager::find(std::span<const RenderPassHandle> handles) const
    {
        std::vector<VkRenderPass> render_passes(handles.size());
        std::ranges::transform(handles, render_passes.begin(), [this](auto handle) { return find(handle); });
        return render_passes;
    }

    std::vector<VkFramebuffer> VulkanResourceManager::find(std::span<const FramebufferHandle> handles) const
    {
        std::vector<VkFramebuffer> framebuffers(handles.size());
        std::ranges::transform(handles, framebuffers.begin(), [this](auto handle) { return find(handle); });
        return framebuffers;
    }

    std::vector<VkShaderModule> VulkanResourceManager::find(std::span<const ShaderModuleHandle> handles) const
    {
        std::vector<VkShaderModule> shaders(handles.size());
        std::ranges::transform(handles, shaders.begin(), [this](auto handle) { return find(handle); });
        return shaders;
    }

    std::vector<VkDescriptorSetLayout> VulkanResourceManager::find(std::span<const DescriptorLayoutHandle> handles) const
    {
        std::vector<VkDescriptorSetLayout> descriptor_layouts(handles.size());
        std::ranges::transform(handles, descriptor_layouts.begin(), [this](auto handle) { return find(handle); });
        return descriptor_layouts;
    }

    std::vector<VkDescriptorSet> VulkanResourceManager::find(std::span<const DescriptorHandle> handles) const
    {
        std::vector<VkDescriptorSet> descriptors(handles.size());
        std::ranges::transform(handles, descriptors.begin(), [this](auto handle) { return find(handle); });
        return descriptors;
    }

    std::vector<VkPipelineLayout> VulkanResourceManager::find(std::span<const PipelineLayoutHandle> handles) const
    {
        std::vector<VkPipelineLayout> pipeline_layouts(handles.size());
        std::ranges::transform(handles, pipeline_layouts.begin(), [this](auto handle) { return find(handle); });
        return pipeline_layouts;
    }

    std::vector<VkPipeline> VulkanResourceManager::find(std::span<const PipelineHandle> handles) const
    {
        std::vector<VkPipeline> pipelines(handles.size());
        std::ranges::transform(handles, pipelines.begin(), [this](auto handle) { return find(handle); });
        return pipelines;
    }

    std::vector<VulkanBufferResource> VulkanResourceManager::find(std::span<const GPUBufferHandle> handles) const
    {
        std::vector<VulkanBufferResource> buffers(handles.size());
        std::ranges::transform(handles, buffers.begin(), [this](auto handle) { return find(handle); });
        return buffers;
    }

    std::vector<VkSampler> VulkanResourceManager::find(std::span<const SamplerHandle> handles) const
    {
        std::vector<VkSampler> samplers(handles.size());
        std::ranges::transform(handles, samplers.begin(), [this](auto handle) { return find(handle); });
        return samplers;
    }

    std::vector<VkFence> VulkanResourceManager::find(std::span<const FenceHandle> handles) const
    {
        std::vector<VkFence> fences(handles.size());
        std::ranges::transform(handles, fences.begin(), [this](auto handle) { return find(handle); });
        return fences;
    }

    std::vector<VkSemaphore> VulkanResourceManager::find(std::span<const SemaphoreHandle> handles) const
    {
        std::vector<VkSemaphore> semaphores(handles.size());
        std::ranges::transform(handles, semaphores.begin(), [this](auto handle) { return find(handle); });
        return semaphores;
    }
} // namespace orion::vulkan
