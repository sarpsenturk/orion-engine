#pragma once

#include "orion/renderapi/render_device.h"

#include <volk.h>

namespace orion
{
    class VulkanDevice final : public RenderDevice
    {
    public:
        VulkanDevice(VkDevice device, VkPhysicalDevice physical_device, VkQueue queue);
        ~VulkanDevice() override;

    private:
        std::unique_ptr<CommandQueue> create_command_queue_api() override;

        VkDevice device_;
        VkPhysicalDevice physical_device_;
        VkQueue queue_;
    };
} // namespace orion
