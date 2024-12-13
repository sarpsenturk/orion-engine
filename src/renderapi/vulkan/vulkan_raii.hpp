#pragma once

#include "vulkan_buffer.hpp"
#include "vulkan_image.hpp"

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

#include <memory>

namespace orion
{
    struct VkDeviceDeleter {
        using pointer = VkDevice;
        void operator()(VkDevice device) const;
    };
    using UniqueVkDevice = std::unique_ptr<VkDevice, VkDeviceDeleter>;

    struct VmaAllocatorDeleter {
        using pointer = VmaAllocator;
        void operator()(VmaAllocator allocator) const;
    };
    using UniqueVmaAllocator = std::unique_ptr<VmaAllocator, VmaAllocatorDeleter>;

    struct VkDescriptorSetLayoutDeleter {
        using pointer = VkDescriptorSetLayout;
        VkDevice device;
        void operator()(VkDescriptorSetLayout descriptor_set_layout) const;
    };
    using UniqueVkDescriptorSetLayout = std::unique_ptr<VkDescriptorSetLayout, VkDescriptorSetLayoutDeleter>;

    struct VkPipelineLayoutDeleter {
        using pointer = VkPipelineLayout;
        VkDevice device;
        void operator()(VkPipelineLayout pipeline_layout) const;
    };
    using UniqueVkPipelineLayout = std::unique_ptr<VkPipelineLayout, VkPipelineLayoutDeleter>;

    struct VkPipelineDeleter {
        using pointer = VkPipeline;
        VkDevice device;
        void operator()(VkPipeline pipeline) const;
    };
    using UniqueVkPipeline = std::unique_ptr<VkPipeline, VkPipelineDeleter>;

    struct VulkanBufferDeleter {
        using pointer = VulkanBuffer;
        VmaAllocator allocator;
        void operator()(VulkanBuffer buffer) const;
    };
    using UniqueVulkanBuffer = std::unique_ptr<VulkanBuffer, VulkanBufferDeleter>;

    struct VulkanImageDeleter {
        using pointer = VulkanImage;
        VmaAllocator allocator;
        void operator()(VulkanImage image) const;
    };
    using UniqueVulkanImage = std::unique_ptr<VulkanImage, VulkanImageDeleter>;

    struct VkImageViewDeleter {
        using pointer = VkImageView;
        VkDevice device;
        void operator()(VkImageView image_view) const;
    };
    using UniqueVkImageView = std::unique_ptr<VkImageView, VkImageViewDeleter>;

    struct VkSemaphoreDeleter {
        using pointer = VkSemaphore;
        VkDevice device;
        void operator()(VkSemaphore semaphore) const;
    };
    using UniqueVkSemaphore = std::unique_ptr<VkSemaphore, VkSemaphoreDeleter>;

    struct VkFenceDeleter {
        using pointer = VkFence;
        VkDevice device;
        void operator()(VkFence fence) const;
    };
    using UniqueVkFence = std::unique_ptr<VkFence, VkFenceDeleter>;

    struct VkDescriptorPoolDeleter {
        using pointer = VkDescriptorPool;
        VkDevice device;
        void operator()(VkDescriptorPool pool) const;
    };
    using UniqueVkDescriptorPool = std::unique_ptr<VkDescriptorPool, VkDescriptorPoolDeleter>;

    struct VkDescriptorSetDeleter {
        using pointer = VkDescriptorSet;
        VkDevice device;
        VkDescriptorPool pool;
        void operator()(VkDescriptorSet descriptor_set) const;
    };
    using UniqueVkDescriptorSet = std::unique_ptr<VkDescriptorSet, VkDescriptorSetDeleter>;

    struct VkSamplerDeleter {
        using pointer = VkSampler;
        VkDevice device;
        void operator()(VkSampler sampler) const;
    };
    using UniqueVkSampler = std::unique_ptr<VkSampler, VkSamplerDeleter>;
} // namespace orion
