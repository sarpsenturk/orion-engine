#pragma once

#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_device.h"

#include "vulkan_headers.h"
#include "vulkan_resource.h"
#include "vulkan_synchronization.h"
#include "vulkan_types.h"

#include <vector>

namespace orion::vulkan
{
    class VulkanDevice final : public RenderDevice
    {
    public:
        VulkanDevice(spdlog::logger* logger, VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues);

        [[nodiscard]] auto vk_device() const noexcept { return device_.get(); }
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
        [[nodiscard]] VkSwapchainKHR create_vk_swapchain(const VulkanSwapchainDesc& desc);

        [[nodiscard]] VulkanResourceManager* resource_manager() { return &resource_manager_; }
        [[nodiscard]] const VulkanResourceManager* resource_manager() const { return &resource_manager_; }

    private:
        [[nodiscard]] UniqueVmaAllocator create_vma_allocator(VkInstance instance, VkPhysicalDevice physical_device) const;
        [[nodiscard]] UniqueVkDescriptorPool create_descriptor_pool() const;
        [[nodiscard]] UniqueVkPipelineLayout create_empty_pipeline_layout() const;

        [[nodiscard]] VkQueue get_queue(CommandQueueType queue_type) const;
        [[nodiscard]] std::uint32_t get_queue_family(CommandQueueType queue_type) const;

        [[nodiscard]] std::vector<std::uint32_t> get_unique_queue_families(const std::vector<CommandQueueType>& queue_types) const;

        [[nodiscard]] VkDescriptorSetLayout create_vk_descriptor_set_layout(const DescriptorLayoutDesc& desc) const;
        [[nodiscard]] VkDescriptorSet create_vk_descriptor_set(VkDescriptorSetLayout descriptor_set_layout) const;

        // Interface Overrides
        std::unique_ptr<CommandAllocator> create_command_allocator_api(const CommandAllocatorDesc& desc) override;
        std::unique_ptr<Swapchain> create_swapchain_api(const Window& window, const SwapchainDesc& desc) override;
        RenderPassHandle create_render_pass_api(const RenderPassDesc& desc) override;
        FramebufferHandle create_framebuffer_api(const FramebufferDesc& desc) override;
        ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc) override;
        DescriptorLayoutHandle create_descriptor_layout_api(const DescriptorLayoutDesc& desc) override;
        DescriptorHandle create_descriptor_api(DescriptorLayoutHandle descriptor_layout_handle) override;
        PipelineLayoutHandle create_pipeline_layout_api(const PipelineLayoutDesc& push_constant) override;
        PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) override;
        GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc) override;
        ImageHandle create_image_api(const ImageDesc& desc) override;
        ImageViewHandle create_image_view_api(const ImageViewDesc& desc) override;
        SamplerHandle create_sampler_api(const SamplerDesc& desc) override;
        FenceHandle create_fence_api(const FenceDesc& desc) override;
        SemaphoreHandle create_semaphore_api() override;

        void destroy_api(RenderPassHandle render_pass_handle) override;
        void destroy_api(FramebufferHandle framebuffer_handle) override;
        void destroy_api(ShaderModuleHandle shader_module_handle) override;
        void destroy_api(DescriptorLayoutHandle descriptor_layout_handle) override;
        void destroy_api(DescriptorHandle descriptor_handle) override;
        void destroy_api(PipelineLayoutHandle pipeline_layout_handle) override;
        void destroy_api(PipelineHandle graphics_pipeline_handle) override;
        void destroy_api(GPUBufferHandle buffer_handle) override;
        void destroy_api(ImageHandle image_handle) override;
        void destroy_api(ImageViewHandle image_view_handle) override;
        void destroy_api(SamplerHandle sampler_handle) override;
        void destroy_api(FenceHandle fence_handle) override;
        void destroy_api(SemaphoreHandle semaphore_handle) override;
        void destroy_flush_api() override;

        void* map_api(GPUBufferHandle buffer_handle) override;
        void unmap_api(GPUBufferHandle buffer_handle) override;

        void wait_for_fences_api(std::span<const FenceHandle> fence_handles) override;
        void wait_queue_idle_api(CommandQueueType queue_type) override;
        void wait_idle_api() override;

        void write_descriptor_api(DescriptorHandle descriptor_handle, std::span<const DescriptorBinding> bindings) override;

        void submit_api(const SubmitDesc& desc, FenceHandle signal_fence) override;
        void* map_api(ImageHandle image_handle) override;
        void unmap_api(ImageHandle image_handle) override;

        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        UniqueVkDevice device_;
        VulkanQueues queues_;
        UniqueVmaAllocator vma_allocator_;

        UniqueVkDescriptorPool descriptor_pool_;
        UniqueVkPipelineLayout empty_pipeline_layout_;
        VulkanResourceManager resource_manager_;
    };
} // namespace orion::vulkan
