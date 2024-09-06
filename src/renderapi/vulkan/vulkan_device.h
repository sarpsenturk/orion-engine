#pragma once

#include "orion/renderapi/render_device.h"

#include "vulkan_handle.h"

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

#include <memory>

namespace orion
{
    class VulkanDevice final : public RenderDevice
    {
    public:
        VulkanDevice(VkDevice device, VkInstance instance, VkPhysicalDevice physical_device, VkQueue queue, std::uint32_t queue_family_index);

    private:
        std::unique_ptr<CommandQueue> create_command_queue_api() override;
        std::unique_ptr<Swapchain> create_swapchain_api(const SwapchainDesc& desc) override;
        std::unique_ptr<ShaderCompiler> create_shader_compiler_api() override;
        std::unique_ptr<CommandAllocator> create_command_allocator_api(const CommandAllocatorDesc& desc) override;
        std::unique_ptr<CommandList> create_command_list_api(const CommandListDesc& desc) override;

        PipelineLayoutHandle create_pipeline_layout_api(const PipelineLayoutDesc& desc) override;
        PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) override;
        BufferHandle create_buffer_api(const BufferDesc& desc) override;

        void destroy_api(PipelineLayoutHandle pipeline_layout) override;
        void destroy_api(PipelineHandle pipeline) override;
        void destroy_api(BufferHandle buffer) override;

        VkShaderModule create_vk_shader_module(std::span<const std::byte> code);

        UniqueVulkanDevice device_;
        UniqueVMAAllocator vma_allocator_;
        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        VkQueue queue_;
        std::uint32_t queue_family_index_;

        VulkanPipelineLayoutTable pipeline_layouts_;
        VulkanPipelineTable pipelines_;
        VulkanBufferTable buffers_;
    };
} // namespace orion
