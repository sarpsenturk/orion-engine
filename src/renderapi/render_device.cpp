#include "orion/renderapi/render_device.h"

#include <spdlog/spdlog.h>

namespace orion
{
    std::unique_ptr<CommandQueue> RenderDevice::create_command_queue()
    {
        auto queue = create_command_queue_api();
        SPDLOG_DEBUG("Created command queue {}", fmt::ptr(queue.get()));
        return queue;
    }

    std::unique_ptr<Swapchain> RenderDevice::create_swapchain(const SwapchainDesc& desc)
    {
        auto swapchain = create_swapchain_api(desc);
        SPDLOG_DEBUG("Created swapchain {}", fmt::ptr(swapchain.get()));
        return swapchain;
    }

    std::unique_ptr<ShaderCompiler> RenderDevice::create_shader_compiler()
    {
        auto compiler = create_shader_compiler_api();
        SPDLOG_DEBUG("Created shader compiler {}", fmt::ptr(compiler.get()));
        return compiler;
    }

    std::unique_ptr<CommandAllocator> RenderDevice::create_command_allocator(const CommandAllocatorDesc& desc)
    {
        auto command_allocator = create_command_allocator_api(desc);
        SPDLOG_DEBUG("Created command allocator {}", fmt::ptr(command_allocator.get()));
        return command_allocator;
    }

    std::unique_ptr<CommandList> RenderDevice::create_command_list(const CommandListDesc& desc)
    {
        auto command_list = create_command_list_api(desc);
        SPDLOG_DEBUG("Created command list {}", fmt::ptr(command_list.get()));
        return command_list;
    }

    PipelineLayoutHandle RenderDevice::create_pipeline_layout(const PipelineLayoutDesc& desc)
    {
        auto pipeline_layout = create_pipeline_layout_api(desc);
        SPDLOG_DEBUG("Created pipeline layout {}", fmt::underlying(pipeline_layout));
        return pipeline_layout;
    }

    PipelineHandle RenderDevice::create_graphics_pipeline(const GraphicsPipelineDesc& desc)
    {
        auto graphics_pipeline = create_graphics_pipeline_api(desc);
        SPDLOG_DEBUG("Created graphics pipeline {}", fmt::underlying(graphics_pipeline));
        return graphics_pipeline;
    }

    BufferHandle RenderDevice::create_buffer(const BufferDesc& desc)
    {
        auto buffer = create_buffer_api(desc);
        SPDLOG_DEBUG("Created buffer {}", fmt::underlying(buffer));
        return buffer;
    }

    void RenderDevice::destroy(PipelineLayoutHandle pipeline_layout)
    {
        destroy_api(pipeline_layout);
        SPDLOG_DEBUG("Destroyed pipeline layout {}", fmt::underlying(pipeline_layout));
    }

    void RenderDevice::destroy(PipelineHandle pipeline)
    {
        destroy_api(pipeline);
        SPDLOG_DEBUG("Destroyed graphics pipeline {}", fmt::underlying(pipeline));
    }

    void RenderDevice::destroy(BufferHandle buffer)
    {
        destroy_api(buffer);
        SPDLOG_DEBUG("Destroyed buffer {}", fmt::underlying(buffer));
    }
} // namespace orion
