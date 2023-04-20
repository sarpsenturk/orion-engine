#include "orion-renderapi/render_device.h"

#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    Swapchain RenderDevice::create_swapchain(const Window& window, SwapchainDesc desc)
    {
        auto handle = create_swapchain_api(window, desc, SwapchainHandle::invalid_handle());
        SPDLOG_DEBUG("Created swapchain with handle {}", handle);
        return {handle, desc};
    }

    ShaderModule RenderDevice::create_shader_module(const ShaderModuleDesc& desc)
    {
        auto handle = create_shader_module_api(desc, ShaderModuleHandle::invalid_handle());
        SPDLOG_DEBUG("Created shader module with handle {}", handle);
        return {handle};
    }

    GraphicsPipeline RenderDevice::create_graphics_pipeline(const GraphicsPipelineDesc& desc)
    {
        auto handle = create_graphics_pipeline_api(desc, PipelineHandle::invalid_handle());
        SPDLOG_DEBUG("Created graphics pipeline with handle {}", handle);
        return {handle};
    }

    GPUBuffer RenderDevice::create_buffer(const GPUBufferDesc& desc)
    {
        auto handle = create_buffer_api(desc, GPUBufferHandle::invalid_handle());
        SPDLOG_DEBUG("Created gpu buffer with handle {}", handle);
        return {handle, desc};
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
} // namespace orion
