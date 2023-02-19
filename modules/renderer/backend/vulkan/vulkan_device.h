#pragma once

#include "orion-renderapi/render_device.h"
#include "vulkan_headers.h"
#include "vulkan_types.h"

namespace orion::vulkan
{
    class VulkanDevice : public RenderDevice
    {
    public:
        VulkanDevice(VkDevice device, VulkanQueues queues);
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice(VulkanDevice&& other) noexcept;
        VulkanDevice& operator=(const VulkanDevice&) = delete;
        VulkanDevice& operator=(VulkanDevice&&) noexcept;
        ~VulkanDevice() override;

    private:
        VkDevice device_;
        VulkanQueues queues_;
    };
} // namespace orion::vulkan
