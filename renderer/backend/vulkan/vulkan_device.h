#pragma once

#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_device.h"
#include "vulkan_buffer.h"
#include "vulkan_headers.h"
#include "vulkan_pipeline.h"
#include "vulkan_swapchain.h"
#include "vulkan_types.h"

#include <unordered_map> // std::unordered_map
#include <vector>        // std::vector

namespace orion::vulkan
{
    struct FindFramebufferResult {
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkSemaphore available_semaphore = VK_NULL_HANDLE;
    };

    struct VulkanSubmission {
        UniqueVkFence fence = VK_NULL_HANDLE;
        UniqueVkSemaphore semaphore = VK_NULL_HANDLE;
        std::vector<VkSemaphore> wait_semaphores;
    };

    class VulkanDevice final : public RenderDevice
    {
    public:
        VulkanDevice(spdlog::logger* logger, VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues);
        ~VulkanDevice();
        VulkanDevice(const VulkanDevice&) = default;
        VulkanDevice(VulkanDevice&&) noexcept = default;
        VulkanDevice& operator=(const VulkanDevice&) = default;
        VulkanDevice& operator=(VulkanDevice&&) noexcept = default;

        [[nodiscard]] VkShaderModule find_shader(ShaderModuleHandle shader_module_handle) const;
        [[nodiscard]] VulkanSwapchain& find_swapchain(SwapchainHandle swapchain_handle);
        [[nodiscard]] VulkanBuffer& find_buffer(GPUBufferHandle buffer_handle);
        [[nodiscard]] VkRenderPass find_render_pass(RenderTargetHandle render_target_handle);
        [[nodiscard]] FindFramebufferResult find_framebuffer(RenderTargetHandle render_target_handle);
        [[nodiscard]] VkPipeline find_pipeline(PipelineHandle pipeline_handle) const;
        [[nodiscard]] VkCommandBuffer find_command_buffer(CommandBufferHandle command_buffer_handle) const;
        [[nodiscard]] VulkanSubmission& find_submission(SubmissionHandle submission_handle);
        [[nodiscard]] VkFence find_fence(SubmissionHandle submission_handle) const;
        [[nodiscard]] VkSemaphore find_semaphore(SubmissionHandle submission_handle) const;

        [[nodiscard]] auto device() const noexcept { return device_.get(); }
        [[nodiscard]] auto allocator() const noexcept { return allocator_.get(); }

    private:
        [[nodiscard]] inline bool transfer_requires_concurrent(std::uint32_t family_index) const noexcept { return queues_.transfer.index != family_index; }

        [[nodiscard]] VkCommandPool graphics_command_pool() const { return command_pools_.at(queues_.graphics.index).get(); }
        [[nodiscard]] VkCommandPool transfer_command_pool() const { return command_pools_.at(queues_.transfer.index).get(); }

        [[nodiscard]] VkQueue graphics_queue() const noexcept { return queues_.graphics.queue; }
        [[nodiscard]] VkQueue transfer_queue() const noexcept { return queues_.transfer.queue; }
        [[nodiscard]] VkQueue compute_queue() const noexcept { return queues_.compute.queue; }

        [[nodiscard]] VkCommandPool get_command_pool(CommandQueueType queue_type) const;
        [[nodiscard]] VkQueue get_queue(CommandQueueType queue_type) const;

        // Vulkan command translation functions
        void compile_commands(VkCommandBuffer command_buffer, const std::vector<CommandPacket>& commands, VulkanSubmission& submission);

        void cmd_buffer_copy(VkCommandBuffer command_buffer, const void* data);
        VkSemaphore cmd_begin_frame(VkCommandBuffer command_buffer, const void* data);
        void cmd_end_frame(VkCommandBuffer command_buffer, const void* data);
        void cmd_draw(VkCommandBuffer command_buffer, const void* data);
        void cmd_draw_indexed(VkCommandBuffer command_buffer, const void* data);

        // Interface Overrides
        SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc) override;
        ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc) override;
        PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) override;
        GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc) override;
        CommandBufferHandle create_command_buffer_api(const CommandBufferDesc& desc) override;

        void recreate_api(SwapchainHandle swapchain_handle, const SwapchainDesc& desc) override;

        void destroy_api(SwapchainHandle swapchain_handle) override;
        void destroy_api(ShaderModuleHandle shader_module_handle) override;
        void destroy_api(PipelineHandle graphics_pipeline_handle) override;
        void destroy_api(GPUBufferHandle buffer_handle) override;
        void destroy_api(CommandBufferHandle command_buffer_handle) override;
        void destroy_api(SubmissionHandle submission_handle) override;

        void* map_api(GPUBufferHandle buffer_handle) override;
        void unmap_api(GPUBufferHandle buffer_handle) override;

        SubmissionHandle submit_api(const CommandBuffer& command_buffer, SubmissionHandle existing) override;
        void wait_api(SubmissionHandle submission_handle) override;
        void present_api(SwapchainHandle swapchain_handle, SubmissionHandle wait) override;

        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        UniqueVkDevice device_;
        UniqueVmaAllocator allocator_ = VK_NULL_HANDLE;
        VulkanQueues queues_;

        std::unordered_map<SwapchainHandle, UniqueVkSurfaceKHR> surfaces_;
        std::unordered_map<SwapchainHandle, VulkanSwapchain> swapchains_;
        std::unordered_map<ShaderModuleHandle, UniqueVkShaderModule> shader_modules_;
        std::unordered_map<PipelineHandle, VulkanPipeline> pipelines_;
        std::unordered_map<GPUBufferHandle, VulkanBuffer> buffers_;
        std::unordered_map<std::uint32_t, UniqueVkCommandPool> command_pools_;
        std::unordered_map<CommandBufferHandle, UniqueVkCommandBuffer> command_buffers_;
        std::unordered_map<SubmissionHandle, VulkanSubmission> submissions_;
    };
} // namespace orion::vulkan
