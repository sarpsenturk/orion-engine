#include "orion-renderapi/render_device.h"

#include <orion-utils/assertion.h> // ORION_ASSERT
#include <spdlog/spdlog.h>         // SPDLOG_LOGGER_*

namespace orion
{
    RenderDevice::RenderDevice(spdlog::logger* logger)
        : logger_(logger)
    {
    }

    SwapchainHandle RenderDevice::create_swapchain(const Window& window, const SwapchainDesc& desc)
    {
        auto handle = create_swapchain_api(window, desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created swapchain with handle {}", handle);
        return handle;
    }

    RenderPassHandle RenderDevice::create_render_pass(const RenderPassDesc& desc)
    {
        auto handle = create_render_pass_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created render pass with handle {}", handle);
        return handle;
    }

    RenderTargetHandle RenderDevice::create_render_target(SwapchainHandle swapchain, const RenderTargetDesc& desc)
    {
        auto handle = create_render_target_api(swapchain, desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created render target with handle {}", handle);
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

    CommandPoolHandle RenderDevice::create_command_pool(const CommandPoolDesc& desc)
    {
        auto handle = create_command_pool_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created command pool with handle {}", handle);
        return handle;
    }

    CommandBufferHandle RenderDevice::create_command_buffer(const CommandBufferDesc& desc)
    {
        auto handle = create_command_buffer_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created command buffer with handle {}", handle);
        return handle;
    }

    DescriptorPoolHandle RenderDevice::create_descriptor_pool(const DescriptorPoolDesc& desc)
    {
        auto handle = create_descriptor_pool_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created descriptor pool with handle {}", handle);
        return handle;
    }

    void RenderDevice::recreate(SwapchainHandle swapchain_handle, const SwapchainDesc& desc)
    {
        recreate_api(swapchain_handle, desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Recreated swapchain with handle {}", swapchain_handle);
    }

    void RenderDevice::recreate(RenderTargetHandle render_target, SwapchainHandle swapchain, const RenderTargetDesc& desc)
    {
        recreate_api(render_target, swapchain, desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Recreated render target with handle {}", render_target);
    }

    void RenderDevice::destroy(SwapchainHandle swapchain_handle)
    {
        destroy_api(swapchain_handle);
    }

    void RenderDevice::destroy(RenderPassHandle render_pass_handle)
    {
        destroy_api(render_pass_handle);
    }

    void RenderDevice::destroy(RenderTargetHandle render_target_handle)
    {
        destroy_api(render_target_handle);
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

    void RenderDevice::destroy(CommandPoolHandle command_pool_handle)
    {
        destroy_api(command_pool_handle);
    }

    void RenderDevice::destroy(CommandBufferHandle command_buffer_handle)
    {
        destroy_api(command_buffer_handle);
    }

    void RenderDevice::destroy(SubmissionHandle submission_handle)
    {
        destroy_api(submission_handle);
    }

    void RenderDevice::destroy(DescriptorPoolHandle descriptor_pool_handle)
    {
        destroy_api(descriptor_pool_handle);
    }

    void* RenderDevice::map(GPUBufferHandle buffer_handle)
    {
        return map_api(buffer_handle);
    }

    void RenderDevice::unmap(GPUBufferHandle buffer_handle)
    {
        return unmap_api(buffer_handle);
    }

    SubmissionHandle RenderDevice::submit(const SubmitDesc& desc)
    {
        return submit_api(desc);
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
