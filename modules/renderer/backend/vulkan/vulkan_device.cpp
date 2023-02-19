#include "vulkan_device.h"

#include <spdlog/spdlog.h> // SPDLOG_*
#include <utility>         // std::exchange

namespace orion::vulkan
{
    VulkanDevice::VulkanDevice(UniqueVkDevice device, VulkanQueues queues)
        : device_(std::move(device))
        , queues_(queues)
    {
    }
} // namespace orion::vulkan
