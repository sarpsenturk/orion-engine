#pragma once

#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_device.h"

#include "orion-utils/static_vector.h"

#include "vulkan_headers.h"
#include "vulkan_store.h"
#include "vulkan_synchronization.h"
#include "vulkan_types.h"

#include <orion-core/window.h>
#include <vector>

namespace orion::vulkan
{
    class VulkanDevice final : public RenderDevice
    {
    public:
        VulkanDevice(spdlog::logger* logger, VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues);

        [[nodiscard]] auto device() const noexcept { return device_.get(); }
        [[nodiscard]] auto vma_allocator() const noexcept { return vma_allocator_.get(); }

        [[nodiscard]] ShaderObjectType shader_object_type() const noexcept override { return ShaderObjectType::SpirV; }

        [[nodiscard]] VkQueue graphics_queue() const noexcept { return queues_.graphics.queue; }
        [[nodiscard]] VkQueue transfer_queue() const noexcept { return queues_.transfer.queue; }
        [[nodiscard]] VkQueue compute_queue() const noexcept { return queues_.compute.queue; }
        [[nodiscard]] std::uint32_t graphics_queue_family() const noexcept { return queues_.graphics.family; }
        [[nodiscard]] std::uint32_t transfer_queue_family() const noexcept { return queues_.transfer.family; }
        [[nodiscard]] std::uint32_t compute_queue_family() const noexcept { return queues_.compute.family; }

        [[nodiscard]] VkSemaphore create_vk_semaphore();
        [[nodiscard]] VkFence create_vk_fence(bool signaled);
        [[nodiscard]] VkCommandPool create_vk_command_pool(std::uint32_t queue_family, VkCommandPoolCreateFlags flags = 0);
        [[nodiscard]] VkCommandBuffer create_vk_command_buffer(VkCommandPool command_pool, VkCommandBufferLevel level);
        [[nodiscard]] VkSwapchainKHR create_vk_swapchain(const VulkanSwapchainDesc& desc);

    private:
        [[nodiscard]] VkQueue get_queue(CommandQueueType queue_type) const;
        [[nodiscard]] std::uint32_t get_queue_family(CommandQueueType queue_type) const;

        [[nodiscard]] std::vector<std::uint32_t> get_unique_queue_families(const std::vector<CommandQueueType>& queue_types) const;

        [[nodiscard]] VkRenderPass create_vkrender_pass(const AttachmentList& attachment_list) const;

        // Interface Overrides
        std::unique_ptr<Swapchain> create_swapchain_api(const SwapchainDesc& desc) override;
        RenderPassHandle create_render_pass_api(const RenderPassDesc& desc) override;
        FramebufferHandle create_framebuffer_api(const FramebufferDesc& desc) override;
        ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc) override;
        PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) override;
        GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc) override;
        CommandPoolHandle create_command_pool_api(const CommandPoolDesc& desc) override;
        CommandBufferHandle create_command_buffer_api(const CommandBufferDesc& desc) override;
        ImageHandle create_image_api(const ImageDesc& desc) override;
        ImageViewHandle create_image_view_api(const ImageViewDesc& desc) override;
        SamplerHandle create_sampler_api(const SamplerDesc& desc) override;
        GPUJobHandle create_job_api(const GPUJobDesc& desc) override;

        void destroy_api(RenderPassHandle render_pass_handle) override;
        void destroy_api(FramebufferHandle framebuffer_handle) override;
        void destroy_api(ShaderModuleHandle shader_module_handle) override;
        void destroy_api(PipelineHandle graphics_pipeline_handle) override;
        void destroy_api(GPUBufferHandle buffer_handle) override;
        void destroy_api(CommandPoolHandle command_pool_handle) override;
        void destroy_api(CommandBufferHandle command_buffer_handle) override;
        void destroy_api(ImageHandle image_handle) override;
        void destroy_api(ImageViewHandle image_view_handle) override;
        void destroy_api(SamplerHandle sampler_handle) override;
        void destroy_api(GPUJobHandle job_handle) override;

        void* map_api(GPUBufferHandle buffer_handle) override;
        void unmap_api(GPUBufferHandle buffer_handle) override;

        void reset_command_pool_api(CommandPoolHandle command_pool) override;
        void reset_command_buffer_api(CommandBufferHandle command_buffer) override;
        void compile_commands_api(CommandBufferHandle command_buffer, std::span<const CommandPacket> commands) override;

        void wait_for_job_api(GPUJobHandle job_handle) override;
        void wait_for_jobs_api(std::span<const GPUJobHandle> job_handles) override;
        void wait_queue_idle_api(CommandQueueType queue_type) override;
        void wait_idle_api() override;

        DrawState draw_state_;
        void reset_draw_state();
        void update_draw_state(VkCommandBuffer command_buffer, const DrawState& new_state);

        void compile_command(VkCommandBuffer command_buffer, const CommandPacket& command_packet);
        void cmd_copy_buffer(VkCommandBuffer command_buffer, const void* data);
        void cmd_begin_render_pass(VkCommandBuffer command_buffer, const void* data);
        void cmd_end_render_pass(VkCommandBuffer command_buffer, const void* data);
        void cmd_draw(VkCommandBuffer command_buffer, const void* data);
        void cmd_draw_indexed(VkCommandBuffer command_buffer, const void* data);
        void cmd_pipeline_barrier(VkCommandBuffer command_buffer, const void* data);
        void cmd_blit_image(VkCommandBuffer command_buffer, const void* data);
        void cmd_push_constants(VkCommandBuffer command_buffer, const void* data);
        void cmd_copy_buffer_to_image(VkCommandBuffer command_buffer, const void* data);

        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        UniqueVkDevice device_;
        UniqueVmaAllocator vma_allocator_;
        VulkanQueues queues_;

        std::unordered_map<std::size_t, VmaAllocation> allocations_;
        std::unordered_map<GPUJobHandle, VulkanJob> jobs_;

        VulkanStore<ImageHandle, UniqueVkImage> images_;
        VulkanStore<ImageViewHandle, UniqueVkImageView> image_views_;
        VulkanStore<RenderPassHandle, UniqueVkRenderPass> render_passes_;
        VulkanStore<FramebufferHandle, UniqueVkFramebuffer> framebuffers_;
        VulkanStore<ShaderModuleHandle, UniqueVkShaderModule> shader_modules_;
        VulkanStore<PipelineHandle, UniqueVkPipelineLayout> pipeline_layouts_;
        VulkanStore<PipelineHandle, UniqueVkPipeline> pipelines_;
        VulkanStore<GPUBufferHandle, UniqueVkBuffer> buffers_;
        VulkanStore<CommandPoolHandle, UniqueVkCommandPool> command_pools_;
        VulkanStore<CommandBufferHandle, UniqueVkCommandBuffer> command_buffers_;
        VulkanStore<SamplerHandle, UniqueVkSampler> samplers_;
    };
} // namespace orion::vulkan
