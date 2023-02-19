#pragma once

#include "orion-renderapi/render_device.h"
#include "vulkan_headers.h"
#include "vulkan_types.h"

namespace orion::vulkan
{
    class VulkanDevice : public RenderDevice
    {
    public:
        VulkanDevice(UniqueVkDevice device, VulkanQueues queues);

    private:
        UniqueVkDevice device_;
        VulkanQueues queues_;
    };
} // namespace orion::vulkan
