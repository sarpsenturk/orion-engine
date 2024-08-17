#include "vulkan_device.h"

namespace orion
{
    VulkanDevice::VulkanDevice(VkDevice device)
        : device_(device)
    {
    }

    VulkanDevice::~VulkanDevice()
    {
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyDevice(device_, nullptr);
        }
    }
} // namespace orion
