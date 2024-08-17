#pragma once

#include "orion/renderapi/render_backend.h"

#include <volk.h>

namespace orion
{
    class VulkanBackend final : public RenderBackend
    {
    public:
        VulkanBackend();
        ~VulkanBackend() override;

        [[nodiscard]] const char* name() const noexcept override { return "Vulkan"; }

    private:
        VkInstance instance_ = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
    };
} // namespace orion
