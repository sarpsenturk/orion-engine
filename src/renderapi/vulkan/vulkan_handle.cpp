#include "vulkan_handle.h"

namespace orion
{
    void VulkanDeviceDeleter::operator()(VkDevice device) const
    {
        vkDestroyDevice(device, nullptr);
    }

    void VMAAllocatorDeleter::operator()(VmaAllocator allocator) const
    {
        vmaDestroyAllocator(allocator);
    }

    void VulkanPipelineLayoutDeleter::operator()(VkDevice device, VkPipelineLayout pipelineLayout) const
    {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void VulkanPipelineDeleter::operator()(VkDevice device, VkPipeline pipeline) const
    {
        vkDestroyPipeline(device, pipeline, nullptr);
    }

    void VulkanBufferDeleter::operator()(VkDevice, VulkanBuffer* buffer) const
    {
        vmaDestroyBuffer(buffer->allocator, buffer->buffer, buffer->allocation);
        delete buffer; // This is ugly but probably the best option for now
    }
} // namespace orion
