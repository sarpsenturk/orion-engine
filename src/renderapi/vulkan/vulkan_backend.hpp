#pragma once

#include "orion/renderapi/render_backend.hpp"

#include <Volk/volk.h>

#include <vector>

namespace orion
{
    class VulkanBackend final : public RenderBackend
    {
    public:
        VulkanBackend();
        ~VulkanBackend() override;

        [[nodiscard]] const char* name() const noexcept override { return "Vulkan"; }

    private:
        std::vector<GraphicsAdapter> get_adapters_api() override;
        std::unique_ptr<RenderDevice> create_device_api(std::size_t adapter_index) override;

        VkInstance instance_ = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
        std::vector<VkPhysicalDevice> physical_devices_;
    };
} // namespace orion
