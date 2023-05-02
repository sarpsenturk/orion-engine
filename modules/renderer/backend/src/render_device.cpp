#include "orion-renderapi/render_device.h"

#include <orion-utils/assertion.h> // ORION_ASSERT
#include <spdlog/spdlog.h>         // SPDLOG_LOGGER_*

namespace orion
{
    RenderDevice::RenderDevice(spdlog::logger* logger)
        : logger_(logger)
    {
    }

    Swapchain RenderDevice::create_swapchain(const Window& window, SwapchainDesc desc)
    {
        auto handle = create_swapchain_api(window, desc, SwapchainHandle::invalid_handle());
        SPDLOG_LOGGER_DEBUG(logger(), "Created swapchain with handle {}", handle);
        return {handle, desc};
    }

    ShaderModule RenderDevice::create_shader_module(const ShaderModuleDesc& desc)
    {
        auto handle = create_shader_module_api(desc, ShaderModuleHandle::invalid_handle());
        SPDLOG_LOGGER_DEBUG(logger(), "Created shader module with handle {}", handle);
        return {handle};
    }

    GraphicsPipeline RenderDevice::create_graphics_pipeline(const GraphicsPipelineDesc& desc)
    {
        auto handle = create_graphics_pipeline_api(desc, PipelineHandle::invalid_handle());
        SPDLOG_LOGGER_DEBUG(logger(), "Created graphics pipeline with handle {}", handle);
        return {handle};
    }

    GPUBuffer RenderDevice::create_buffer(const GPUBufferDesc& desc)
    {
        auto handle = create_buffer_api(desc, GPUBufferHandle::invalid_handle());
        SPDLOG_LOGGER_DEBUG(logger(), "Created gpu buffer with handle {}", handle);
        return {handle, desc};
    }

    CommandBuffer RenderDevice::create_command_buffer(const CommandBufferDesc& desc, std::unique_ptr<CommandAllocator> allocator)
    {
        return {create_command_buffer_api(desc, CommandBufferHandle::invalid_handle()), desc, std::move(allocator)};
    }

    void RenderDevice::destroy(Swapchain swapchain)
    {
        destroy_api(swapchain.handle());
    }

    void RenderDevice::destroy(ShaderModule shader_module)
    {
        destroy_api(shader_module.handle());
    }

    void RenderDevice::destroy(GraphicsPipeline graphics_pipeline)
    {
        destroy_api(graphics_pipeline.handle());
    }

    void RenderDevice::destroy(GPUBuffer buffer)
    {
        destroy_api(buffer.handle());
    }

    void RenderDevice::destroy(const CommandBuffer& command_buffer)
    {
        destroy_api(command_buffer.handle());
    }

    void RenderDevice::destroy(SubmissionHandle submission_handle)
    {
        destroy_api(submission_handle);
    }

    void* RenderDevice::map(GPUBuffer buffer)
    {
        ORION_EXPECTS(buffer.host_visible());
        return map_api(buffer.handle());
    }

    void RenderDevice::unmap(GPUBuffer buffer)
    {
        return unmap_api(buffer.handle());
    }

    SubmissionHandle RenderDevice::submit(const CommandBuffer& command_buffer, SubmissionHandle existing)
    {
        return submit_api(command_buffer, existing);
    }

    void RenderDevice::wait(SubmissionHandle submission_handle)
    {
        if (submission_handle.is_valid()) {
            wait_api(submission_handle);
        }
    }

    void RenderDevice::present(Swapchain swapchain, SubmissionHandle wait)
    {
        present_api(swapchain.handle(), wait);
    }
} // namespace orion
