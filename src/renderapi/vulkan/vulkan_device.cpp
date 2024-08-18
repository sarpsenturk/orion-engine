#include "vulkan_device.h"

#include "vulkan_queue.h"

namespace orion
{
    VulkanDevice::VulkanDevice(VkDevice device, VkPhysicalDevice physical_device, VkQueue queue)
        : device_(device)
        , physical_device_(physical_device)
        , queue_(queue)
    {
    }

    VulkanDevice::~VulkanDevice()
    {
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyDevice(device_, nullptr);
        }
    }

    std::unique_ptr<CommandQueue> VulkanDevice::create_command_queue_api()
    {
        return std::make_unique<VulkanQueue>(queue_);
    }
} // namespace orion
