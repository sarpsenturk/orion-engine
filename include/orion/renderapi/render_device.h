#pragma once

#include "buffer.h"
#include "orion/renderapi/handle.h"
#include "orion/renderapi/pipeline.h"
#include "orion/renderapi/render_command.h"
#include "orion/renderapi/render_queue.h"
#include "orion/renderapi/shader.h"
#include "orion/renderapi/swapchain.h"
#include "orion/renderapi/sync.h"

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

        PipelineLayoutHandle create_pipeline_layout(const PipelineLayoutDesc& desc);
        PipelineHandle create_graphics_pipeline(const GraphicsPipelineDesc& desc);
        BufferHandle create_buffer(const BufferDesc& desc);
        SemaphoreHandle create_semaphore(const SemaphoreDesc& desc);
        FenceHandle create_fence(const FenceDesc& desc);

        void destroy(PipelineLayoutHandle pipeline_layout);
        void destroy(PipelineHandle pipeline);
        void destroy(BufferHandle buffer);
        void destroy(SemaphoreHandle semaphore);
        void destroy(FenceHandle fence);

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

        virtual PipelineLayoutHandle create_pipeline_layout_api(const PipelineLayoutDesc& desc) = 0;
        virtual PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) = 0;
        virtual BufferHandle create_buffer_api(const BufferDesc& desc) = 0;
        virtual SemaphoreHandle create_semaphore_api(const SemaphoreDesc& desc) = 0;
        virtual FenceHandle create_fence_api(const FenceDesc& desc) = 0;

        virtual void destroy_api(PipelineHandle pipeline) = 0;
        virtual void destroy_api(PipelineLayoutHandle pipeline_layout) = 0;
        virtual void destroy_api(BufferHandle buffer) = 0;
        virtual void destroy_api(SemaphoreHandle semaphore) = 0;
        virtual void destroy_api(FenceHandle fence) = 0;

        virtual void* map_api(BufferHandle buffer) = 0;
        virtual void unmap_api(BufferHandle buffer) = 0;

        virtual void wait_for_fence_api(FenceHandle fence) = 0;
    };
} // namespace orion
