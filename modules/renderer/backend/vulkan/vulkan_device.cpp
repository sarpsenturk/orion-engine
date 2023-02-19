#include "vulkan_device.h"

#include <spdlog/spdlog.h> // SPDLOG_*
#include <utility>         // std::exchange

namespace orion::vulkan
{
    VulkanDevice::VulkanDevice(VkDevice device, VulkanQueues queues)
        : device_(device)
        , queues_(queues)
    {
    }

    VulkanDevice::VulkanDevice(VulkanDevice&& other) noexcept
        : device_(std::exchange(other.device_, VK_NULL_HANDLE))
        , queues_(other.queues_)
    {
    }

    VulkanDevice& VulkanDevice::operator=(VulkanDevice&& other) noexcept
    {
        device_ = std::exchange(other.device_, VK_NULL_HANDLE);
        queues_ = other.queues_;
        return *this;
    }

    VulkanDevice::~VulkanDevice()
    {
        if (device_) {
            vkDestroyDevice(device_, alloc_callbacks());
            SPDLOG_TRACE("Destroyed VkDevice {}", fmt::ptr(device_));
        }
    }
} // namespace orion::vulkan
