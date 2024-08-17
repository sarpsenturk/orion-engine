#pragma once

#include "orion/renderapi/render_device.h"

#include <volk.h>

namespace orion
{
    class VulkanDevice final : public RenderDevice
    {
    public:
        VulkanDevice(VkDevice device);
        ~VulkanDevice() override;

    private:
        VkDevice device_;
    };
} // namespace orion
