#pragma once

#include "orion-renderer/render_backend.h"

namespace orion::vulkan
{
    class VulkanBackend : public RenderBackend
    {
    public:
        [[nodiscard]] const char* name() const noexcept override { return "Vulkan 1.0"; }
    };
} // namespace orion::vulkan
