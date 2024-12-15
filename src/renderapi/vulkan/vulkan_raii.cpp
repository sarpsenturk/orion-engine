#include "vulkan_raii.hpp"

#include "orion/assertion.hpp"

namespace orion
{
    void VkDeviceDeleter::operator()(VkDevice device) const
    {
        vkDestroyDevice(device, nullptr);
    }

    void VmaAllocatorDeleter::operator()(VmaAllocator allocator) const
    {
        vmaDestroyAllocator(allocator);
    }

    void VulkanDescriptorSetLayoutDeleter::operator()(VulkanDescriptorSetLayout descriptor_set_layout) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE);
        vkDestroyDescriptorSetLayout(device, descriptor_set_layout.layout, nullptr);
        vkDestroyDescriptorPool(device, descriptor_set_layout.pool, nullptr);
    }

    void VkPipelineLayoutDeleter::operator()(VkPipelineLayout pipeline_layout) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE);
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    }

    void VkPipelineDeleter::operator()(VkPipeline pipeline) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE);
        vkDestroyPipeline(device, pipeline, nullptr);
    }

    void VulkanBufferDeleter::operator()(VulkanBuffer buffer) const
    {
        ORION_ASSERT(allocator != VK_NULL_HANDLE);
        vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
    }

    void VulkanImageDeleter::operator()(VulkanImage image) const
    {
        ORION_ASSERT(allocator != VK_NULL_HANDLE);
        if (image.is_user_image()) {
            vmaDestroyImage(allocator, image.image, image.allocation);
        }
    }

    void VkImageViewDeleter::operator()(VkImageView image_view) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE);
        vkDestroyImageView(device, image_view, nullptr);
    }

    void VkSemaphoreDeleter::operator()(VkSemaphore semaphore) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE);
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    void VkFenceDeleter::operator()(VkFence fence) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE);
        vkDestroyFence(device, fence, nullptr);
    }

    void VkDescriptorPoolDeleter::operator()(VkDescriptorPool pool) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE);
        vkDestroyDescriptorPool(device, pool, nullptr);
    }

    void VkDescriptorSetDeleter::operator()(VkDescriptorSet descriptor_set) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE);
        ORION_ASSERT(pool != VK_NULL_HANDLE);
        vkFreeDescriptorSets(device, pool, 1, &descriptor_set);
    }

    void VkSamplerDeleter::operator()(VkSampler sampler) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE);
        vkDestroySampler(device, sampler, nullptr);
    }
} // namespace orion
