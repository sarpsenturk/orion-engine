#pragma once

#include "orion/renderapi/render_device.h"

#include "vulkan_context.h"

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

#include <memory>

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
        std::unique_ptr<CommandAllocator> create_command_allocator_api(const CommandAllocatorDesc& desc) override;
        std::unique_ptr<CommandList> create_command_list_api(const CommandListDesc& desc) override;

        DescriptorSetLayoutHandle create_descriptor_set_layout_api(const DescriptorSetLayoutDesc& desc) override;
        PipelineLayoutHandle create_pipeline_layout_api(const PipelineLayoutDesc& desc) override;
        PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) override;
        BufferHandle create_buffer_api(const BufferDesc& desc) override;
        SemaphoreHandle create_semaphore_api(const SemaphoreDesc& desc) override;
        FenceHandle create_fence_api(const FenceDesc& desc) override;
        DescriptorPoolHandle create_descriptor_pool_api(const DescriptorPoolDesc& desc) override;
        DescriptorSetHandle create_descriptor_set_api(const DescriptorSetDesc& desc) override;

        void destroy_api(DescriptorSetLayoutHandle descriptor_set_layout) override;
        void destroy_api(PipelineLayoutHandle pipeline_layout) override;
        void destroy_api(PipelineHandle pipeline) override;
        void destroy_api(BufferHandle buffer) override;
        void destroy_api(SemaphoreHandle semaphore) override;
        void destroy_api(FenceHandle fence) override;
        void destroy_api(DescriptorPoolHandle descriptor_pool) override;
        void destroy_api(DescriptorSetHandle descriptor_set) override;

        void* map_api(BufferHandle buffer) override;
        void unmap_api(BufferHandle buffer) override;

        void wait_for_fence_api(FenceHandle fence) override;

        VkShaderModule create_vk_shader_module(std::span<const std::byte> code);

        VkDevice device_;
        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        VkQueue queue_;
        std::uint32_t queue_family_index_;

        VmaAllocator vma_allocator_;
        VulkanContext context_;
    };
} // namespace orion
