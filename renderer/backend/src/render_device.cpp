#include "orion-renderapi/render_device.h"

#include <orion-utils/assertion.h> // ORION_ASSERT
#include <spdlog/spdlog.h>         // SPDLOG_LOGGER_*

namespace orion
{
    RenderDevice::RenderDevice(spdlog::logger* logger)
        : logger_(logger)
    {
    }

    SwapchainHandle RenderDevice::create_swapchain(const Window& window, SwapchainDesc desc)
    {
        auto handle = create_swapchain_api(window, desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created swapchain with handle {}", handle);
        return handle;
    }

    ShaderModuleHandle RenderDevice::create_shader_module(const ShaderModuleDesc& desc)
    {
        auto handle = create_shader_module_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created shader module with handle {}", handle);
        return handle;
    }

    PipelineHandle RenderDevice::create_graphics_pipeline(const GraphicsPipelineDesc& desc)
    {
        auto handle = create_graphics_pipeline_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created graphics pipeline with handle {}", handle);
        return handle;
    }

    GPUBufferHandle RenderDevice::create_buffer(const GPUBufferDesc& desc)
    {
        auto handle = create_buffer_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created gpu buffer with handle {}", handle);
        return handle;
    }

    CommandBuffer RenderDevice::create_command_buffer(const CommandBufferDesc& desc, std::unique_ptr<CommandAllocator> allocator)
    {
        return {create_command_buffer_api(desc), desc, std::move(allocator)};
    }

    void RenderDevice::recreate(SwapchainHandle swapchain_handle, const SwapchainDesc& desc)
    {
        return recreate_api(swapchain_handle, desc);
    }

    void RenderDevice::destroy(SwapchainHandle swapchain_handle)
    {
        destroy_api(swapchain_handle);
    }

    void RenderDevice::destroy(ShaderModuleHandle shader_module_handle)
    {
        destroy_api(shader_module_handle);
    }

    void RenderDevice::destroy(PipelineHandle pipeline_handle)
    {
        destroy_api(pipeline_handle);
    }

    void RenderDevice::destroy(GPUBufferHandle buffer_handle)
    {
        destroy_api(buffer_handle);
    }

    void RenderDevice::destroy(CommandBufferHandle command_buffer_handle)
    {
        destroy_api(command_buffer_handle);
    }

    void RenderDevice::destroy(SubmissionHandle submission_handle)
    {
        destroy_api(submission_handle);
    }

    void* RenderDevice::map(GPUBufferHandle buffer_handle)
    {
        return map_api(buffer_handle);
    }

    void RenderDevice::unmap(GPUBufferHandle buffer_handle)
    {
        return unmap_api(buffer_handle);
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

    void RenderDevice::present(SwapchainHandle swapchain_handle, SubmissionHandle wait)
    {
        present_api(swapchain_handle, wait);
    }
} // namespace orion