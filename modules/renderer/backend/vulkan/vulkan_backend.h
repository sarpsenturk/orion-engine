#pragma once

#include "orion-core/config.h"
#include "orion-renderer/render_backend.h"
#include "vulkan_headers.h"

namespace orion::vulkan
{
    class VulkanBackend : public RenderBackend
    {
    public:
        VulkanBackend();
        VulkanBackend(const VulkanBackend&) = delete;
        VulkanBackend(VulkanBackend&& other) noexcept;
        VulkanBackend& operator=(const VulkanBackend&) = delete;
        VulkanBackend& operator=(VulkanBackend&& other) noexcept;
        ~VulkanBackend() override;

        [[nodiscard]] const char* name() const noexcept override { return "Vulkan 1.0"; }

        static constexpr auto vulkan_api_version = VK_API_VERSION_1_0;

    private:
        VkInstance instance_ = VK_NULL_HANDLE;
    };
} // namespace orion::vulkan
