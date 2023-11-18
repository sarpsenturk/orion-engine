#include "orion-renderapi/render_device.h"

#include <spdlog/spdlog.h>

namespace orion
{
    RenderDevice::RenderDevice(spdlog::logger* logger)
        : logger_(logger)
    {
    }

    std::unique_ptr<CommandAllocator> RenderDevice::create_command_allocator(orion::CommandQueueType queue_type)
    {
        auto allocator = create_command_allocator_api(queue_type);
        SPDLOG_LOGGER_DEBUG(logger(), "Created command allocator interface at {}", fmt::ptr(allocator));
        return allocator;
    }

    std::unique_ptr<Swapchain> RenderDevice::create_swapchain(const SwapchainDesc& desc)
    {
        ORION_EXPECTS(desc.window != nullptr);
        auto swapchain = create_swapchain_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created swapchain interface at {}", fmt::ptr(swapchain));
        return swapchain;
    }

    RenderPassHandle RenderDevice::create_render_pass(const RenderPassDesc& desc)
    {
        auto handle = create_render_pass_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created render pass with handle {}", handle);
        return handle;
    }

    FramebufferHandle RenderDevice::create_framebuffer(const FramebufferDesc& desc)
    {
        auto handle = create_framebuffer_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created framebuffer with handle {}", handle);
        return handle;
    }

    ShaderModuleHandle RenderDevice::create_shader_module(const ShaderModuleDesc& desc)
    {
        auto handle = create_shader_module_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created shader module with handle {}", handle);
        return handle;
    }

    PipelineLayoutHandle RenderDevice::create_pipeline_layout(const PipelineLayoutDesc& desc)
    {
        auto handle = create_pipeline_layout_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created pipeline layout with handle {}", handle);
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

    ImageHandle RenderDevice::create_image(const ImageDesc& desc)
    {
        auto handle = create_image_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created image with handle {}", handle);
        return handle;
    }

    ImageViewHandle RenderDevice::create_image_view(const ImageViewDesc& desc)
    {
        auto handle = create_image_view_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created image view with handle {}", handle);
        return handle;
    }

    SamplerHandle RenderDevice::create_sampler(const SamplerDesc& desc)
    {
        auto handle = create_sampler_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created sampler {}", handle);
        return handle;
    }

    GPUJobHandle RenderDevice::create_job(const GPUJobDesc& desc)
    {
        auto handle = create_job_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created GPU job {}", handle);
        return handle;
    }

    void RenderDevice::destroy(RenderPassHandle render_pass_handle)
    {
        destroy_api(render_pass_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed render pass {}", render_pass_handle);
    }

    void RenderDevice::destroy(FramebufferHandle framebuffer_handle)
    {
        destroy_api(framebuffer_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed framebuffer {}", framebuffer_handle);
    }

    void RenderDevice::destroy(ShaderModuleHandle shader_module_handle)
    {
        destroy_api(shader_module_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed shader module {}", shader_module_handle);
    }

    void RenderDevice::destroy(PipelineLayoutHandle pipeline_layout_handle)
    {
        destroy_api(pipeline_layout_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed pipeline layout {}", pipeline_layout_handle);
    }

    void RenderDevice::destroy(PipelineHandle pipeline_handle)
    {
        destroy_api(pipeline_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed pipeline {}", pipeline_handle);
    }

    void RenderDevice::destroy(GPUBufferHandle buffer_handle)
    {
        destroy_api(buffer_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed GPUBuffer {}", buffer_handle);
    }

    void RenderDevice::destroy(ImageHandle image_handle)
    {
        destroy_api(image_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed image {}", image_handle);
    }

    void RenderDevice::destroy(ImageViewHandle image_view_handle)
    {
        destroy_api(image_view_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed image view {}", image_view_handle);
    }

    void RenderDevice::destroy(SamplerHandle sampler_handle)
    {
        destroy_api(sampler_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed sampler {}", sampler_handle);
    }

    void RenderDevice::destroy(GPUJobHandle job_handle)
    {
        destroy_api(job_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed GPU job {}", job_handle);
    }

    void* RenderDevice::map(GPUBufferHandle buffer_handle)
    {
        return map_api(buffer_handle);
    }

    void RenderDevice::unmap(GPUBufferHandle buffer_handle)
    {
        return unmap_api(buffer_handle);
    }

    void RenderDevice::wait_for_job(GPUJobHandle job_handle)
    {
        wait_for_job_api(job_handle);
    }

    void RenderDevice::wait_for_jobs(std::span<const GPUJobHandle> job_handles)
    {
        wait_for_jobs_api(job_handles);
    }

    void RenderDevice::wait_queue_idle(CommandQueueType queue_type)
    {
        wait_queue_idle_api(queue_type);
    }

    void RenderDevice::wait_idle()
    {
        wait_idle_api();
    }
} // namespace orion
