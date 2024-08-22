#pragma once

#include "orion/renderapi/render_device.h"

#include "vulkan_handle.h"

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

        GraphicsPipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) override;

        void destroy_api(GraphicsPipelineHandle pipeline) override;

        VkShaderModule create_vk_shader_module(std::span<const std::byte> code);

        VkDevice device_;
        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        VkQueue queue_;
        std::uint32_t queue_family_index_;

        VulkanHandleTable handle_table_;
    };
} // namespace orion
