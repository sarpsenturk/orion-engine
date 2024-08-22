#pragma once

#include "orion/renderapi/render_device.h"

#include <Volk/volk.h>

namespace orion
{
    class VulkanDevice final : public RenderDevice
    {
    public:
        VulkanDevice(VkDevice device, VkInstance instance, VkPhysicalDevice physical_device, VkQueue queue, std::uint32_t queue_family_index);
        ~VulkanDevice() override;

    private:
        std::unique_ptr<CommandQueue> create_command_queue_api() override;
        std::unique_ptr<Swapchain> create_swapchain_api(const SwapchainDesc& desc) override;
        std::unique_ptr<ShaderCompiler> create_shader_compiler_api() override;

        VkDevice device_;
        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        VkQueue queue_;
        std::uint32_t queue_family_index_;
    };
} // namespace orion
