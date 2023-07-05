#pragma once

#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_device.h"
#include "vulkan_buffer.h"
#include "vulkan_headers.h"
#include "vulkan_pipeline.h"
#include "vulkan_types.h"

#include <unordered_map> // std::unordered_map
#include <vector>        // std::vector

namespace orion::vulkan
{
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
        [[nodiscard]] VkRenderPass find_render_pass(RenderPassHandle render_pass_handle);
        [[nodiscard]] VulkanRenderTarget& find_render_target(RenderTargetHandle render_target_handle);
        [[nodiscard]] VkPipeline find_pipeline(PipelineHandle pipeline_handle) const;
        [[nodiscard]] VkCommandPool find_command_pool(CommandPoolHandle command_pool_handle) const;
        [[nodiscard]] VkCommandBuffer find_command_buffer(CommandBufferHandle command_buffer_handle) const;
        [[nodiscard]] VulkanSubmission& find_submission(SubmissionHandle submission_handle);
        [[nodiscard]] VkFence find_fence(SubmissionHandle submission_handle) const;
        [[nodiscard]] VkSemaphore find_semaphore(SubmissionHandle submission_handle) const;
        [[nodiscard]] VkDescriptorPool find_descriptor_pool(DescriptorPoolHandle descriptor_pool_handle) const;
        [[nodiscard]] VkDescriptorSet find_descriptor_set(DescriptorSetHandle descriptor_set_handle) const;

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
        VkDescriptorSetLayout create_descriptor_set_layout(std::span<const DescriptorBinding> bindings);

        // Vulkan command translation functions
        void compile_commands(VkCommandBuffer command_buffer, const std::vector<CommandPacket>& commands, VulkanSubmission& submission);

        void cmd_buffer_copy(VkCommandBuffer command_buffer, const void* data);
        VkSemaphore cmd_begin_frame(VkCommandBuffer command_buffer, const void* data);
        void cmd_end_frame(VkCommandBuffer command_buffer, const void* data);
        void cmd_draw(VkCommandBuffer command_buffer, const void* data);
        void cmd_draw_indexed(VkCommandBuffer command_buffer, const void* data);

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
        void destroy_api(SubmissionHandle submission_handle) override;
        void destroy_api(DescriptorPoolHandle descriptor_pool_handle) override;
        void destroy_api(DescriptorSetHandle descriptor_set_handle) override;

        void* map_api(GPUBufferHandle buffer_handle) override;
        void unmap_api(GPUBufferHandle buffer_handle) override;

        SubmissionHandle submit_api(const SubmitDesc& desc) override;
        void wait_api(SubmissionHandle submission_handle) override;
        void present_api(SwapchainHandle swapchain_handle, SubmissionHandle wait) override;

        void update_descriptors_api(const DescriptorUpdate& update) override;

        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        UniqueVkDevice device_;
        UniqueVmaAllocator allocator_;
        VulkanQueues queues_;

        std::unordered_map<SwapchainHandle, UniqueVkSurfaceKHR> surfaces_;
        std::unordered_map<SwapchainHandle, VulkanSwapchain> swapchains_;
        std::unordered_map<RenderPassHandle, UniqueVkRenderPass> render_passes_;
        std::unordered_map<RenderTargetHandle, VulkanRenderTarget> render_targets_;
        std::unordered_map<ShaderModuleHandle, UniqueVkShaderModule> shader_modules_;
        std::unordered_map<PipelineHandle, VulkanPipeline> pipelines_;
        std::unordered_map<GPUBufferHandle, VulkanBuffer> buffers_;
        std::unordered_map<CommandPoolHandle, UniqueVkCommandPool> command_pools_;
        std::unordered_map<CommandBufferHandle, UniqueVkCommandBuffer> command_buffers_;
        std::unordered_map<SubmissionHandle, VulkanSubmission> submissions_;
        std::unordered_map<std::size_t, UniqueVkDescriptorSetLayout> descriptor_set_layout_cache_;
        std::unordered_map<DescriptorPoolHandle, UniqueVkDescriptorPool> descriptor_pools_;
        std::unordered_map<DescriptorSetHandle, UniqueVkDescriptorSet> descriptor_sets_;
    };
} // namespace orion::vulkan
