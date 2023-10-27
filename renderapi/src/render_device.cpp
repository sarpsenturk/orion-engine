#include "orion-renderapi/render_device.h"

#include <spdlog/spdlog.h>

namespace orion
{
    RenderDevice::RenderDevice(spdlog::logger* logger)
        : logger_(logger)
    {
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

    DescriptorSetHandle RenderDevice::create_descriptor_set(const DescriptorSetDesc& desc)
    {
        auto handle = create_descriptor_set_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Allocated descriptor set with handle {}", handle);
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

    void RenderDevice::destroy(CommandPoolHandle command_pool_handle)
    {
        destroy_api(command_pool_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed command pool {}", command_pool_handle);
    }

    void RenderDevice::destroy(CommandBufferHandle command_buffer_handle)
    {
        destroy_api(command_buffer_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed command buffer {}", command_buffer_handle);
    }

    void RenderDevice::destroy(DescriptorPoolHandle descriptor_pool_handle)
    {
        destroy_api(descriptor_pool_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed descriptor pool {}", descriptor_pool_handle);
    }

    void RenderDevice::destroy(DescriptorSetHandle descriptor_set_handle)
    {
        destroy_api(descriptor_set_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed descriptor set {}", descriptor_set_handle);
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

    void RenderDevice::reset_command_pool(CommandPoolHandle command_pool)
    {
        reset_command_pool_api(command_pool);
    }

    void RenderDevice::reset_command_buffer(CommandBufferHandle command_buffer)
    {
        reset_command_buffer_api(command_buffer);
    }

    void RenderDevice::compile_commands(CommandBufferHandle command_buffer, std::span<const CommandPacket> commands)
    {
        compile_commands_api(command_buffer, commands);
    }

    void RenderDevice::wait_for_job(GPUJobHandle job_handle)
    {
        wait_for_job_api(job_handle);
    }

    void RenderDevice::wait_queue_idle(CommandQueueType queue_type)
    {
        wait_queue_idle_api(queue_type);
    }

    void RenderDevice::wait_idle()
    {
        wait_idle_api();
    }

    void RenderDevice::update_descriptor_sets(std::span<const DescriptorSetUpdate> updates)
    {
        update_descriptor_sets_api(updates);
    }
} // namespace orion
