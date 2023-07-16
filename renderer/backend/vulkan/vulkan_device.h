#pragma once

#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_device.h"

#include "vulkan_buffer.h"
#include "vulkan_headers.h"
#include "vulkan_pipeline.h"
#include "vulkan_store.h"
#include "vulkan_types.h"

#include <vector>

namespace orion::vulkan
{
    class VulkanDevice final : public RenderDevice
    {
    public:
        VulkanDevice(spdlog::logger* logger, VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues);
        ~VulkanDevice() override;
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice(VulkanDevice&&) noexcept = default;
        VulkanDevice& operator=(const VulkanDevice&) = delete;
        VulkanDevice& operator=(VulkanDevice&&) noexcept = default;

        [[nodiscard]] auto device() const noexcept { return device_.get(); }
        [[nodiscard]] auto allocator() const noexcept { return allocator_.get(); }

    private:
        [[nodiscard]] inline bool transfer_requires_concurrent(std::uint32_t family_index) const noexcept { return queues_.transfer.index != family_index; }

        [[nodiscard]] VkQueue graphics_queue() const noexcept { return queues_.graphics.queue; }
        [[nodiscard]] VkQueue transfer_queue() const noexcept { return queues_.transfer.queue; }
        [[nodiscard]] VkQueue compute_queue() const noexcept { return queues_.compute.queue; }

        [[nodiscard]] VkQueue get_queue(CommandQueueType queue_type) const;
        [[nodiscard]] std::uint32_t get_queue_family(CommandQueueType queue_type) const;

        VkDescriptorSetLayout make_descriptor_set_layout(const DescriptorSetLayout& layout);
        [[nodiscard]] VkDescriptorSetLayout create_descriptor_set_layout(std::span<const DescriptorBinding> bindings) const;

        // Interface Overrides
        SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc) override;
        RenderPassHandle create_render_pass_api(const RenderPassDesc& desc) override;
        RenderTargetHandle create_render_target_api(SwapchainHandle swapchain_handle, const RenderTargetDesc& desc) override;
        ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc) override;
        PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) override;
        GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc) override;
        CommandPoolHandle create_command_pool_api(const CommandPoolDesc& desc) override;
        CommandBufferHandle create_command_buffer_api(const CommandBufferDesc& desc) override;
        DescriptorPoolHandle create_descriptor_pool_api(const DescriptorPoolDesc& desc) override;
        DescriptorSetHandle create_descriptor_set_api(const DescriptorSetDesc& desc) override;
        SemaphoreHandle create_semaphore_api() override;
        FenceHandle create_fence_api(bool create_signaled) override;

        void recreate_api(SwapchainHandle swapchain_handle, const SwapchainDesc& desc) override;
        void recreate_api(RenderTargetHandle render_target, SwapchainHandle swapchain, const RenderTargetDesc& desc) override;

        void destroy_api(SwapchainHandle swapchain_handle) override;
        void destroy_api(RenderPassHandle render_pass_handle) override;
        void destroy_api(RenderTargetHandle render_target_handle) override;
        void destroy_api(ShaderModuleHandle shader_module_handle) override;
        void destroy_api(PipelineHandle graphics_pipeline_handle) override;
        void destroy_api(GPUBufferHandle buffer_handle) override;
        void destroy_api(CommandPoolHandle command_pool_handle) override;
        void destroy_api(CommandBufferHandle command_buffer_handle) override;
        void destroy_api(DescriptorPoolHandle descriptor_pool_handle) override;
        void destroy_api(DescriptorSetHandle descriptor_set_handle) override;
        void destroy_api(SemaphoreHandle semaphore_handle) override;
        void destroy_api(FenceHandle fence_handle) override;

        void* map_api(GPUBufferHandle buffer_handle) override;
        void unmap_api(GPUBufferHandle buffer_handle) override;

        void begin_command_buffer_api(CommandBufferHandle command_buffer, const CommandBufferBeginDesc& desc) override;
        void end_command_buffer_api(CommandBufferHandle command_buffer) override;
        void reset_command_buffer_api(CommandBufferHandle command_buffer) override;
        void compile_commands_api(CommandBufferHandle command_buffer, std::span<const CommandPacket> commands) override;

        void submit_api(const SubmitDesc& desc) override;
        void present_api(SwapchainHandle swapchain_handle, SemaphoreHandle wait_semaphore) override;

        void wait_for_fence_api(FenceHandle fence) override;
        void wait_queue_idle_api(CommandQueueType queue_type) override;
        void wait_idle_api() override;

        void update_descriptors_api(const DescriptorUpdate& update) override;

        void compile_command(VkCommandBuffer command_buffer, const CommandPacket& command_packet);
        void cmd_buffer_copy(VkCommandBuffer command_buffer, const void* data);
        void cmd_begin_frame(VkCommandBuffer command_buffer, const void* data);
        void cmd_end_frame(VkCommandBuffer command_buffer, const void* data);
        void cmd_draw(VkCommandBuffer command_buffer, const void* data);
        void cmd_draw_indexed(VkCommandBuffer command_buffer, const void* data);
        void cmd_bind_descriptor_sets(VkCommandBuffer command_buffer, const void* data);

        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        UniqueVkDevice device_;
        UniqueVmaAllocator allocator_;
        VulkanQueues queues_;
        VulkanStore store_;

        std::unordered_map<std::size_t, UniqueVkDescriptorSetLayout> descriptor_set_layouts_;
    };
} // namespace orion::vulkan
