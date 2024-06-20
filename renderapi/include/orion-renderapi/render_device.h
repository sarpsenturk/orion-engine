#pragma once

#include "orion-renderapi/blend.h"
#include "orion-renderapi/buffer.h"
#include "orion-renderapi/descriptor.h"
#include "orion-renderapi/device_resource.h"
#include "orion-renderapi/handles.h"
#include "orion-renderapi/image.h"
#include "orion-renderapi/render_command.h"
#include "orion-renderapi/render_pass.h"
#include "orion-renderapi/render_queue.h"
#include "orion-renderapi/shader.h"
#include "orion-renderapi/shader_reflection.h"
#include "orion-renderapi/swapchain.h"

#include <memory>
#include <span>

namespace spdlog
{
    class logger;
}

namespace orion
{
    // Forward declarations
    class Window;

    class RenderDevice
    {
    public:
        explicit RenderDevice(spdlog::logger* logger);
        virtual ~RenderDevice() = default;

        [[nodiscard]] virtual ShaderObjectType shader_object_type() const noexcept = 0;
        [[nodiscard]] virtual const char* shader_base_path() const noexcept = 0;

        [[nodiscard]] std::unique_ptr<CommandQueue> create_queue(CommandQueueType type);
        [[nodiscard]] std::unique_ptr<CommandAllocator> create_command_allocator(const CommandAllocatorDesc& desc);
        [[nodiscard]] std::unique_ptr<Swapchain> create_swapchain(CommandQueue* queue, const Window& window, const SwapchainDesc& desc);
        [[nodiscard]] std::unique_ptr<ShaderReflector> create_shader_reflector();
        [[nodiscard]] std::unique_ptr<RenderPass> create_render_pass();
        [[nodiscard]] ShaderModuleHandle create_shader_module(const ShaderModuleDesc& desc);
        [[nodiscard]] DescriptorLayoutHandle create_descriptor_layout(const DescriptorLayoutDesc& desc);
        [[nodiscard]] DescriptorPoolHandle create_descriptor_pool(const DescriptorPoolDesc& desc);
        [[nodiscard]] DescriptorHandle create_descriptor(DescriptorLayoutHandle descriptor_layout_handle, DescriptorPoolHandle descriptor_pool_handle);
        [[nodiscard]] PipelineLayoutHandle create_pipeline_layout(const PipelineLayoutDesc& desc);
        [[nodiscard]] PipelineHandle create_graphics_pipeline(const GraphicsPipelineDesc& desc);
        [[nodiscard]] GPUBufferHandle create_buffer(const GPUBufferDesc& desc);
        [[nodiscard]] ImageHandle create_image(const ImageDesc& desc);
        [[nodiscard]] ImageViewHandle create_image_view(const ImageViewDesc& desc);
        [[nodiscard]] SamplerHandle create_sampler(const SamplerDesc& desc);
        [[nodiscard]] FenceHandle create_fence(const FenceDesc& desc);
        [[nodiscard]] SemaphoreHandle create_semaphore();

        [[nodiscard]] ShaderModuleHandle create(ShaderModuleHandle_tag, const ShaderModuleDesc& desc) { return create_shader_module(desc); }
        [[nodiscard]] DescriptorLayoutHandle create(DescriptorLayoutHandle_tag, const DescriptorLayoutDesc& desc) { return create_descriptor_layout(desc); }
        [[nodiscard]] DescriptorPoolHandle create(DescriptorPoolHandle_tag, const DescriptorPoolDesc& desc) { return create_descriptor_pool(desc); }
        [[nodiscard]] DescriptorHandle create(DescriptorHandle_tag, DescriptorLayoutHandle descriptor_layout_handle, DescriptorPoolHandle descriptor_pool_handle) { return create_descriptor(descriptor_layout_handle, descriptor_pool_handle); }
        [[nodiscard]] PipelineLayoutHandle create(PipelineLayoutHandle_tag, const PipelineLayoutDesc& desc) { return create_pipeline_layout(desc); }
        [[nodiscard]] PipelineHandle create(PipelineHandle_tag, const GraphicsPipelineDesc& desc) { return create_graphics_pipeline(desc); }
        [[nodiscard]] GPUBufferHandle create(GPUBufferHandle_tag, const GPUBufferDesc& desc) { return create_buffer(desc); }
        [[nodiscard]] ImageHandle create(ImageHandle_tag, const ImageDesc& desc) { return create_image(desc); }
        [[nodiscard]] ImageViewHandle create(ImageViewHandle_tag, const ImageViewDesc& desc) { return create_image_view(desc); }
        [[nodiscard]] SamplerHandle create(SamplerHandle_tag, const SamplerDesc& desc) { return create_sampler(desc); }
        [[nodiscard]] FenceHandle create(FenceHandle_tag, const FenceDesc& desc) { return create_fence(desc); }
        [[nodiscard]] SemaphoreHandle create(SemaphoreHandle_tag) { return create_semaphore(); }

        template<typename Tag, typename... Args>
        UniqueDeviceHandle<Tag> make_unique(Args&&... args)
        {
            return {create(Tag{}, std::forward<Args>(args)...), this};
        }

        template<typename Tag>
        UniqueDeviceHandle<Tag> to_unique(RenderDeviceHandle<Tag> handle)
        {
            return {handle, this};
        }

        void destroy(ShaderModuleHandle shader_module_handle);
        void destroy(DescriptorLayoutHandle descriptor_layout_handle);
        void destroy(DescriptorPoolHandle descriptor_pool_handle);
        void destroy(DescriptorHandle descriptor_handle);
        void destroy(PipelineLayoutHandle pipeline_layout_handle);
        void destroy(PipelineHandle pipeline_handle);
        void destroy(GPUBufferHandle buffer_handle);
        void destroy(ImageHandle image_handle);
        void destroy(ImageViewHandle image_view_handle);
        void destroy(SamplerHandle sampler_handle);
        void destroy(FenceHandle fence_handle);
        void destroy(SemaphoreHandle semaphore_handle);
        void destroy_flush();

        [[nodiscard]] void* map(GPUBufferHandle buffer_handle);
        [[nodiscard]] void* map(ImageHandle image_handle);
        void unmap(GPUBufferHandle buffer_handle);
        void unmap(ImageHandle image_handle);

        void wait_for_fence(FenceHandle fence_handle);
        void wait_for_fences(std::span<const FenceHandle> fence_handles);
        void wait_queue_idle(CommandQueueType queue_type);
        void wait_idle();

        void reset_descriptor_pool(DescriptorPoolHandle descriptor_pool_handle);

        void write_descriptor(DescriptorHandle descriptor_handle, std::span<const DescriptorWrite> writes);
        void write_descriptor(DescriptorHandle descriptor_handle, const DescriptorWrite& write);
        void write_descriptor(DescriptorHandle descriptor_handle, std::uint32_t binding, DescriptorType type, const BufferDescriptorDesc& buffer);
        void write_descriptor(DescriptorHandle descriptor_handle, std::uint32_t binding, ImageViewHandle image_view_handle, ImageLayout layout);
        void write_descriptor(DescriptorHandle descriptor_handle, std::uint32_t binding, SamplerHandle sampler);

        [[nodiscard]] auto logger() const noexcept { return logger_; }

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) noexcept = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice& operator=(RenderDevice&&) noexcept = default;

    private:
        [[nodiscard]] virtual std::unique_ptr<CommandQueue> create_queue_api(CommandQueueType type) = 0;
        [[nodiscard]] virtual std::unique_ptr<CommandAllocator> create_command_allocator_api(const CommandAllocatorDesc& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<Swapchain> create_swapchain_api(CommandQueue* queue, const Window& window, const SwapchainDesc& desc) = 0;
        [[nodiscard]] virtual std::unique_ptr<ShaderReflector> create_shader_reflector_api() = 0;
        [[nodiscard]] virtual std::unique_ptr<RenderPass> create_render_pass_api() = 0;
        [[nodiscard]] virtual ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc) = 0;
        [[nodiscard]] virtual DescriptorLayoutHandle create_descriptor_layout_api(const DescriptorLayoutDesc& desc) = 0;
        [[nodiscard]] virtual DescriptorPoolHandle create_descriptor_pool_api(const DescriptorPoolDesc& desc) = 0;
        [[nodiscard]] virtual DescriptorHandle create_descriptor_api(DescriptorLayoutHandle descriptor_layout_handle, DescriptorPoolHandle descriptor_pool_handle) = 0;
        [[nodiscard]] virtual PipelineLayoutHandle create_pipeline_layout_api(const PipelineLayoutDesc& desc) = 0;
        [[nodiscard]] virtual PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) = 0;
        [[nodiscard]] virtual GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc) = 0;
        [[nodiscard]] virtual ImageHandle create_image_api(const ImageDesc& desc) = 0;
        [[nodiscard]] virtual ImageViewHandle create_image_view_api(const ImageViewDesc& desc) = 0;
        [[nodiscard]] virtual SamplerHandle create_sampler_api(const SamplerDesc& desc) = 0;
        [[nodiscard]] virtual FenceHandle create_fence_api(const FenceDesc& desc) = 0;
        [[nodiscard]] virtual SemaphoreHandle create_semaphore_api() = 0;

        virtual void destroy_api(ShaderModuleHandle shader_module_handle) = 0;
        virtual void destroy_api(DescriptorLayoutHandle descriptor_layout_handle) = 0;
        virtual void destroy_api(DescriptorPoolHandle descriptor_pool_handle) = 0;
        virtual void destroy_api(DescriptorHandle descriptor_handle) = 0;
        virtual void destroy_api(PipelineLayoutHandle pipeline_layout_handle) = 0;
        virtual void destroy_api(PipelineHandle graphics_pipeline_handle) = 0;
        virtual void destroy_api(GPUBufferHandle buffer_handle) = 0;
        virtual void destroy_api(ImageHandle image_handle) = 0;
        virtual void destroy_api(ImageViewHandle image_view_handle) = 0;
        virtual void destroy_api(SamplerHandle sampler_handle) = 0;
        virtual void destroy_api(FenceHandle fence_handle) = 0;
        virtual void destroy_api(SemaphoreHandle semaphore_handle) = 0;

        virtual void destroy_flush_api() = 0;

        [[nodiscard]] virtual void* map_api(GPUBufferHandle buffer_handle) = 0;
        [[nodiscard]] virtual void* map_api(ImageHandle image_handle) = 0;
        virtual void unmap_api(GPUBufferHandle buffer_handle) = 0;
        virtual void unmap_api(ImageHandle image_handle) = 0;

        virtual void wait_for_fences_api(std::span<const FenceHandle> fence_handles) = 0;
        virtual void wait_queue_idle_api(CommandQueueType queue_type) = 0;
        virtual void wait_idle_api() = 0;

        virtual void reset_descriptor_pool_api(DescriptorPoolHandle descriptor_pool_handle) = 0;

        virtual void write_descriptor_api(DescriptorHandle descriptor_handle, std::span<const DescriptorWrite> bindings) = 0;

        spdlog::logger* logger_;
    };
} // namespace orion
