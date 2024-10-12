#pragma once

#include "orion/renderapi/handle.h"

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

#include <array>

namespace orion
{
    struct VulkanBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        explicit(false) operator bool() const noexcept { return buffer != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE; }
    };

    struct VulkanDescriptorSet {
        VkDescriptorSet set = VK_NULL_HANDLE;
        VkDescriptorPool pool = VK_NULL_HANDLE;
    };

    struct VulkanImage {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        [[nodiscard]] bool is_user_image() const noexcept { return image != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE; }
    };

    class VulkanContext
    {
    public:
        template<typename T>
        struct TableEntry {
            T value = {};
            std::uint16_t gen = 0;
        };

        template<typename T>
        using ResourceTable = std::array<TableEntry<T>, 0xffff>;

        VulkanContext(VkDevice device, VmaAllocator allocator);
        ~VulkanContext();

        DescriptorSetLayoutHandle insert(VkDescriptorSetLayout descriptor_set_layout);
        PipelineLayoutHandle insert(VkPipelineLayout pipeline_layout);
        PipelineHandle insert(VkPipeline pipeline);
        BufferHandle insert(VkBuffer buffer, VmaAllocation allocation);
        ImageHandle insert(VkImage image, VmaAllocation allocation = VK_NULL_HANDLE);
        RenderTargetHandle insert(VkImageView image_view);
        SemaphoreHandle insert(VkSemaphore semaphore);
        FenceHandle insert(VkFence fence);
        DescriptorPoolHandle insert(VkDescriptorPool descriptor_pool);
        DescriptorSetHandle insert(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool);

        VkDescriptorSetLayout lookup(DescriptorSetLayoutHandle descriptor_set_layout) const;
        VkPipelineLayout lookup(PipelineLayoutHandle pipeline_layout) const;
        VkPipeline lookup(PipelineHandle pipeline) const;
        VulkanBuffer lookup(BufferHandle buffer) const;
        VkImage lookup(ImageHandle image) const;
        VkImageView lookup(RenderTargetHandle render_target) const;
        VkSemaphore lookup(SemaphoreHandle semaphore) const;
        VkFence lookup(FenceHandle fence) const;
        VkDescriptorPool lookup(DescriptorPoolHandle descriptor_pool) const;
        VkDescriptorSet lookup(DescriptorSetHandle descriptor_set) const;

        bool remove(PipelineLayoutHandle pipeline_layout);
        bool remove(PipelineHandle pipeline);
        bool remove(BufferHandle buffer);
        bool remove(ImageHandle image);
        bool remove(RenderTargetHandle render_target);
        bool remove(SemaphoreHandle semaphore);
        bool remove(FenceHandle fence);
        bool remove(DescriptorPoolHandle descriptor_pool);
        bool remove(DescriptorSetLayoutHandle descriptor_set_layout);
        bool remove(DescriptorSetHandle descriptor_set);

    private:
        VkDevice device_;
        VmaAllocator allocator_;

        ResourceTable<VkDescriptorSetLayout> descriptor_set_layouts_;
        ResourceTable<VkPipelineLayout> pipeline_layouts_;
        ResourceTable<VkPipeline> pipelines_;
        ResourceTable<VulkanBuffer> buffers_;
        ResourceTable<VulkanImage> images_;
        ResourceTable<VkImageView> image_views_;
        ResourceTable<VkSemaphore> semaphores_;
        ResourceTable<VkFence> fences_;
        ResourceTable<VkDescriptorPool> descriptor_pools_;
        ResourceTable<VulkanDescriptorSet> descriptor_sets_;
    };
} // namespace orion
