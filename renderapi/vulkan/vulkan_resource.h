#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include "orion-renderapi/handles.h"

#include <span>
#include <unordered_map>
#include <vector>

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

    struct VulkanDescriptorPool {
        VkDescriptorPool descriptor_pool;
        VkDescriptorPoolCreateFlags flags;
    };

    class VulkanResourceManager
    {
    public:
        VulkanResourceManager(VkDevice device, VmaAllocator vma_allocator);

        void add(ImageHandle handle, VkImage image, VmaAllocation allocation);
        void add(ImageHandle handle, VkImage image);
        void add(ImageViewHandle handle, VkImageView image_view);
        void add(ShaderModuleHandle handle, VkShaderModule shader_module);
        void add(DescriptorLayoutHandle handle, VkDescriptorSetLayout descriptor_set_layout);
        void add(DescriptorPoolHandle handle, VkDescriptorPool descriptor_pool, VkDescriptorPoolCreateFlags flags);
        void add(DescriptorHandle handle, VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool, bool free_descriptor_set);
        void add(PipelineLayoutHandle handle, VkPipelineLayout pipeline_layout);
        void add(PipelineHandle handle, VkPipeline pipeline);
        void add(GPUBufferHandle handle, VkBuffer buffer, VmaAllocation allocation);
        void add(SamplerHandle handle, VkSampler sampler);
        void add(FenceHandle handle, VkFence fence);
        void add(SemaphoreHandle handle, VkSemaphore semaphore);

        template<typename HandleType>
        void remove(HandleType handle)
        {
            deletion_queue_.emplace_back(handle.value(), [](VulkanResourceManager* resource_manager, render_device_key_t handle) {
                resource_manager->destroy(HandleType{handle});
            });
        }

        void destroy(ImageHandle handle);
        void destroy(ImageViewHandle handle);
        void destroy(ShaderModuleHandle handle);
        void destroy(DescriptorLayoutHandle handle);
        void destroy(DescriptorPoolHandle handle);
        void destroy(DescriptorHandle handle);
        void destroy(PipelineLayoutHandle handle);
        void destroy(PipelineHandle handle);
        void destroy(GPUBufferHandle handle);
        void destroy(SamplerHandle handle);
        void destroy(FenceHandle handle);
        void destroy(SemaphoreHandle handle);
        void destroy_flush();

        [[nodiscard]] VulkanImageResource find(ImageHandle handle) const noexcept;
        [[nodiscard]] VkImageView find(ImageViewHandle handle) const noexcept;
        [[nodiscard]] VkShaderModule find(ShaderModuleHandle handle) const noexcept;
        [[nodiscard]] VkDescriptorSetLayout find(DescriptorLayoutHandle handle) const noexcept;
        [[nodiscard]] VulkanDescriptorPool find(DescriptorPoolHandle handle) const noexcept;
        [[nodiscard]] VkDescriptorSet find(DescriptorHandle handle) const noexcept;
        [[nodiscard]] VkPipelineLayout find(PipelineLayoutHandle handle) const noexcept;
        [[nodiscard]] VkPipeline find(PipelineHandle handle) const noexcept;
        [[nodiscard]] VulkanBufferResource find(GPUBufferHandle handle) const noexcept;
        [[nodiscard]] VkSampler find(SamplerHandle handle) const noexcept;
        [[nodiscard]] VkFence find(FenceHandle handle) const noexcept;
        [[nodiscard]] VkSemaphore find(SemaphoreHandle handle) const noexcept;

        [[nodiscard]] std::vector<VulkanImageResource> find(std::span<const ImageHandle> handles) const;
        [[nodiscard]] std::vector<VkImageView> find(std::span<const ImageViewHandle> handles) const;
        [[nodiscard]] std::vector<VkShaderModule> find(std::span<const ShaderModuleHandle> handles) const;
        [[nodiscard]] std::vector<VkDescriptorSetLayout> find(std::span<const DescriptorLayoutHandle> handles) const;
        [[nodiscard]] std::vector<VulkanDescriptorPool> find(std::span<const DescriptorPoolHandle> handles) const;
        [[nodiscard]] std::vector<VkDescriptorSet> find(std::span<const DescriptorHandle> handles) const;
        [[nodiscard]] std::vector<VkPipelineLayout> find(std::span<const PipelineLayoutHandle> handles) const;
        [[nodiscard]] std::vector<VkPipeline> find(std::span<const PipelineHandle> handles) const;
        [[nodiscard]] std::vector<VulkanBufferResource> find(std::span<const GPUBufferHandle> handles) const;
        [[nodiscard]] std::vector<VkSampler> find(std::span<const SamplerHandle> handles) const;
        [[nodiscard]] std::vector<VkFence> find(std::span<const FenceHandle> handles) const;
        [[nodiscard]] std::vector<VkSemaphore> find(std::span<const SemaphoreHandle> handles) const;

    private:
        VkDevice device_;
        VmaAllocator vma_allocator_;

        std::unordered_map<ImageHandle, UniqueVkImage> images_;
        std::unordered_map<ImageViewHandle, UniqueVkImageView> image_views_;
        std::unordered_map<ShaderModuleHandle, UniqueVkShaderModule> shader_modules_;
        std::unordered_map<DescriptorLayoutHandle, UniqueVkDescriptorSetLayout> descriptor_set_layouts_;
        std::unordered_map<DescriptorPoolHandle, UniqueVkDescriptorPool> descriptor_pools_;
        std::unordered_map<DescriptorHandle, UniqueVkDescriptorSet> descriptor_sets_;
        std::unordered_map<PipelineLayoutHandle, UniqueVkPipelineLayout> pipeline_layouts_;
        std::unordered_map<PipelineHandle, UniqueVkPipeline> pipelines_;
        std::unordered_map<GPUBufferHandle, UniqueVkBuffer> buffers_;
        std::unordered_map<SamplerHandle, UniqueVkSampler> samplers_;
        std::unordered_map<FenceHandle, UniqueVkFence> fences_;
        std::unordered_map<SemaphoreHandle, UniqueVkSemaphore> semaphores_;

        using DestroyFn = void (*)(VulkanResourceManager*, render_device_key_t);
        struct QueueItem {
            render_device_key_t handle;
            DestroyFn deleter;
        };
        std::vector<QueueItem> deletion_queue_;
    };
} // namespace orion::vulkan
