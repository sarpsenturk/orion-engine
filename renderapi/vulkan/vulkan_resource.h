#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include "orion-renderapi/handles.h"

#include <unordered_map>

namespace orion::vulkan
{
    struct VulkanImageResource {
        VkImage image;
        VmaAllocation allocation;
    };

    struct VulkanBufferResource {
        VkBuffer buffer;
        VmaAllocation allocation;
    };

    class VulkanResourceManager
    {
    public:
        VulkanResourceManager(VkDevice device, VmaAllocator vma_allocator);

        void add(ImageHandle handle, VkImage image, VmaAllocation allocation);
        void add(ImageViewHandle handle, VkImageView image_view);
        void add(RenderPassHandle handle, VkRenderPass render_pass);
        void add(FramebufferHandle handle, VkFramebuffer framebuffer);
        void add(ShaderModuleHandle handle, VkShaderModule shader_module);
        void add(DescriptorLayoutHandle handle, VkDescriptorSetLayout descriptor_set_layout);
        void add(DescriptorHandle handle, VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool);
        void add(PipelineLayoutHandle handle, VkPipelineLayout pipeline_layout);
        void add(PipelineHandle handle, VkPipeline pipeline);
        void add(GPUBufferHandle handle, VkBuffer buffer, VmaAllocation allocation);
        void add(SamplerHandle handle, VkSampler sampler);
        void add(FenceHandle handle, VkFence fence);
        void add(SemaphoreHandle handle, VkSemaphore semaphore);

        void remove(ImageHandle handle);
        void remove(ImageViewHandle handle);
        void remove(RenderPassHandle handle);
        void remove(FramebufferHandle handle);
        void remove(ShaderModuleHandle handle);
        void remove(DescriptorLayoutHandle handle);
        void remove(DescriptorHandle handle);
        void remove(PipelineLayoutHandle handle);
        void remove(PipelineHandle handle);
        void remove(GPUBufferHandle handle);
        void remove(SamplerHandle handle);
        void remove(FenceHandle handle);
        void remove(SemaphoreHandle handle);

        [[nodiscard]] VulkanImageResource find(ImageHandle handle) const noexcept;
        [[nodiscard]] VkImageView find(ImageViewHandle handle) const noexcept;
        [[nodiscard]] VkRenderPass find(RenderPassHandle handle) const noexcept;
        [[nodiscard]] VkFramebuffer find(FramebufferHandle handle) const noexcept;
        [[nodiscard]] VkShaderModule find(ShaderModuleHandle handle) const noexcept;
        [[nodiscard]] VkDescriptorSetLayout find(DescriptorLayoutHandle handle) const noexcept;
        [[nodiscard]] VkDescriptorSet find(DescriptorHandle handle) const noexcept;
        [[nodiscard]] VkPipelineLayout find(PipelineLayoutHandle handle) const noexcept;
        [[nodiscard]] VkPipeline find(PipelineHandle handle) const noexcept;
        [[nodiscard]] VulkanBufferResource find(GPUBufferHandle handle) const noexcept;
        [[nodiscard]] VkSampler find(SamplerHandle handle) const noexcept;
        [[nodiscard]] VkFence find(FenceHandle handle) const noexcept;
        [[nodiscard]] VkSemaphore find(SemaphoreHandle handle) const noexcept;

    private:
        VkDevice device_;
        VmaAllocator vma_allocator_;

        std::unordered_map<ImageHandle, UniqueVkImage> images_;
        std::unordered_map<ImageViewHandle, UniqueVkImageView> image_views_;
        std::unordered_map<RenderPassHandle, UniqueVkRenderPass> render_passes_;
        std::unordered_map<FramebufferHandle, UniqueVkFramebuffer> framebuffers_;
        std::unordered_map<ShaderModuleHandle, UniqueVkShaderModule> shader_modules_;
        std::unordered_map<DescriptorLayoutHandle, UniqueVkDescriptorSetLayout> descriptor_set_layouts_;
        std::unordered_map<DescriptorHandle, UniqueVkDescriptorSet> descriptor_sets_;
        std::unordered_map<PipelineLayoutHandle, UniqueVkPipelineLayout> pipeline_layouts_;
        std::unordered_map<PipelineHandle, UniqueVkPipeline> pipelines_;
        std::unordered_map<GPUBufferHandle, UniqueVkBuffer> buffers_;
        std::unordered_map<SamplerHandle, UniqueVkSampler> samplers_;
        std::unordered_map<FenceHandle, UniqueVkFence> fences_;
        std::unordered_map<SemaphoreHandle, UniqueVkSemaphore> semaphores_;
    };
} // namespace orion::vulkan
