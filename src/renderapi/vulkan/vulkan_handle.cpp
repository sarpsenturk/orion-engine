#include "vulkan_handle.h"

namespace orion
{
    void VulkanDeviceDeleter::operator()(VkDevice device) const
    {
        vkDestroyDevice(device, nullptr);
    }

    void VulkanPipelineLayoutDeleter::operator()(VkDevice device, VkPipelineLayout pipelineLayout) const
    {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void VulkanPipelineDeleter::operator()(VkDevice device, VkPipeline pipeline) const
    {
        vkDestroyPipeline(device, pipeline, nullptr);
    }
} // namespace orion
