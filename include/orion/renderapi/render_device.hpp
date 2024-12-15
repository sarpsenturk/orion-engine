#pragma once

#include "orion/renderapi/bind_group.hpp"
#include "orion/renderapi/buffer.hpp"
#include "orion/renderapi/handle.hpp"
#include "orion/renderapi/image.hpp"
#include "orion/renderapi/pipeline.hpp"
#include "orion/renderapi/render_command.hpp"
#include "orion/renderapi/render_queue.hpp"
#include "orion/renderapi/shader.hpp"
#include "orion/renderapi/swapchain.hpp"
#include "orion/renderapi/sync.hpp"

#include <memory>

namespace orion
{
    class RenderDevice
    {
    public:
        RenderDevice() = default;
        virtual ~RenderDevice() = default;

        std::unique_ptr<CommandQueue> create_command_queue();
        std::unique_ptr<Swapchain> create_swapchain(const SwapchainDesc& desc);
        std::unique_ptr<ShaderCompiler> create_shader_compiler();
        std::unique_ptr<CommandAllocator> create_command_allocator(const CommandAllocatorDesc& desc);
        std::unique_ptr<CommandList> create_command_list(const CommandListDesc& desc);

        BindGroupLayoutHandle create_bind_group_layout(const BindGroupLayoutDesc& desc);
        PipelineLayoutHandle create_pipeline_layout(const PipelineLayoutDesc& desc);
        PipelineHandle create_graphics_pipeline(const GraphicsPipelineDesc& desc);
        BufferHandle create_buffer(const BufferDesc& desc);
        SemaphoreHandle create_semaphore(const SemaphoreDesc& desc);
        FenceHandle create_fence(const FenceDesc& desc);
        BindGroupHandle create_bind_group(const BindGroupDesc& desc);
        ImageHandle create_image(const ImageDesc& desc);

        // TODO: For consistency, consider returning BufferViewHandle from create_constant_buffer_view

        void create_constant_buffer_view(const ConstantBufferViewDesc& desc);
        void create_robuffer_view(const ROBufferViewDesc& desc);
        ImageViewHandle create_image_view(const ImageViewDesc& desc);
        SamplerHandle create_sampler(const SamplerDesc& desc);

        void destroy(BindGroupLayoutHandle bind_group_layout);
        void destroy(PipelineLayoutHandle pipeline_layout);
        void destroy(PipelineHandle pipeline);
        void destroy(BufferHandle buffer);
        void destroy(SemaphoreHandle semaphore);
        void destroy(FenceHandle fence);
        void destroy(BindGroupHandle bind_group);
        void destroy(ImageHandle image);
        void destroy(ImageViewHandle image_view);
        void destroy(SamplerHandle sampler);

        void* map(BufferHandle buffer);
        void unmap(BufferHandle buffer);
        void memcpy(BufferHandle dst, const void* src, std::size_t size);

        void wait_for_fence(FenceHandle fence);

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) = default;
        RenderDevice& operator=(RenderDevice&&) = default;

    private:
        virtual std::unique_ptr<CommandQueue> create_command_queue_api() = 0;
        virtual std::unique_ptr<Swapchain> create_swapchain_api(const SwapchainDesc& desc) = 0;
        virtual std::unique_ptr<ShaderCompiler> create_shader_compiler_api() = 0;
        virtual std::unique_ptr<CommandAllocator> create_command_allocator_api(const CommandAllocatorDesc& desc) = 0;
        virtual std::unique_ptr<CommandList> create_command_list_api(const CommandListDesc& desc) = 0;

        virtual BindGroupLayoutHandle create_bind_group_layout_api(const BindGroupLayoutDesc& desc) = 0;
        virtual PipelineLayoutHandle create_pipeline_layout_api(const PipelineLayoutDesc& desc) = 0;
        virtual PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) = 0;
        virtual BufferHandle create_buffer_api(const BufferDesc& desc) = 0;
        virtual SemaphoreHandle create_semaphore_api(const SemaphoreDesc& desc) = 0;
        virtual FenceHandle create_fence_api(const FenceDesc& desc) = 0;
        virtual BindGroupHandle create_bind_group_api(const BindGroupDesc& desc) = 0;
        virtual ImageHandle create_image_api(const ImageDesc& desc) = 0;

        virtual void create_constant_buffer_view_api(const ConstantBufferViewDesc& desc) = 0;
        virtual void create_robuffer_view_api(const ROBufferViewDesc& desc) = 0;
        virtual ImageViewHandle create_image_view_api(const ImageViewDesc& desc) = 0;
        virtual SamplerHandle create_sampler_api(const SamplerDesc& desc) = 0;

        virtual void destroy_api(BindGroupLayoutHandle descriptor_set_layout) = 0;
        virtual void destroy_api(PipelineLayoutHandle pipeline_layout) = 0;
        virtual void destroy_api(PipelineHandle pipeline) = 0;
        virtual void destroy_api(BufferHandle buffer) = 0;
        virtual void destroy_api(SemaphoreHandle semaphore) = 0;
        virtual void destroy_api(FenceHandle fence) = 0;
        virtual void destroy_api(BindGroupHandle descriptor_set) = 0;
        virtual void destroy_api(ImageHandle image) = 0;
        virtual void destroy_api(ImageViewHandle image_view) = 0;
        virtual void destroy_api(SamplerHandle sampler) = 0;

        virtual void* map_api(BufferHandle buffer) = 0;
        virtual void unmap_api(BufferHandle buffer) = 0;

        virtual void wait_for_fence_api(FenceHandle fence) = 0;
    };
} // namespace orion
