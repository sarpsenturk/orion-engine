#include "orion-renderapi/render_device.h"

#include <spdlog/spdlog.h>

namespace orion
{
    RenderDevice::RenderDevice(spdlog::logger* logger)
        : logger_(logger)
    {
    }

    CommandPoolHandle RenderDevice::default_command_pool()
    {
        if (!default_command_pool_.is_valid()) {
            default_command_pool_ = create_command_pool({.queue_type = CommandQueueType::Any});
        }
        return default_command_pool_;
    }

    CommandBufferHandle RenderDevice::default_command_buffer()
    {
        if (!default_command_buffer_.is_valid()) {
            default_command_buffer_ = create_command_buffer({.command_pool = default_command_pool()});
        }
        return default_command_buffer_;
    }

    FenceHandle RenderDevice::default_fence()
    {
        if (!default_fence_.is_valid()) {
            default_fence_ = create_fence(false);
        }
        return default_fence_;
    }

    SurfaceHandle RenderDevice::create_surface(const Window& window)
    {
        auto handle = create_surface_api(window);
        SPDLOG_LOGGER_DEBUG(logger(), "Create surface {}", handle);
        return handle;
    }

    SwapchainHandle RenderDevice::create_swapchain(const SwapchainDesc& desc)
    {
        auto handle = create_swapchain_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created swapchain with handle {}", handle);
        return handle;
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

    SemaphoreHandle RenderDevice::create_semaphore()
    {
        auto handle = create_semaphore_api();
        SPDLOG_LOGGER_DEBUG(logger(), "Created semaphore with handle {}", handle);
        return handle;
    }

    FenceHandle RenderDevice::create_fence(bool create_signaled)
    {
        auto handle = create_fence_api(create_signaled);
        SPDLOG_LOGGER_DEBUG(logger(), "Created fence with handle {}", handle);
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

    void RenderDevice::destroy(SurfaceHandle surface_handle)
    {
        destroy_api(surface_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed surface {}", surface_handle);
    }

    void RenderDevice::destroy(SwapchainHandle swapchain_handle)
    {
        destroy_api(swapchain_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed swapchain {}", swapchain_handle);
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

    void RenderDevice::destroy(SemaphoreHandle semaphore_handle)
    {
        destroy_api(semaphore_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed semaphore {}", semaphore_handle);
    }

    void RenderDevice::destroy(FenceHandle fence_handle)
    {
        destroy_api(fence_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed fence {}", fence_handle);
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

    void RenderDevice::begin_command_buffer(CommandBufferHandle command_buffer, const CommandBufferBeginDesc& desc)
    {
        begin_command_buffer_api(command_buffer, desc);
    }

    void RenderDevice::end_command_buffer(CommandBufferHandle command_buffer)
    {
        end_command_buffer_api(command_buffer);
    }

    void RenderDevice::reset_command_buffer(CommandBufferHandle command_buffer)
    {
        reset_command_buffer_api(command_buffer);
    }

    void RenderDevice::compile_commands(CommandBufferHandle command_buffer, std::span<const CommandPacket> commands)
    {
        compile_commands_api(command_buffer, commands);
    }

    void RenderDevice::submit(const SubmitDesc& desc)
    {
        submit_api(desc);
    }

    void RenderDevice::present(const SwapchainPresentDesc& desc)
    {
        present_api(desc);
    }

    void RenderDevice::wait_for_fence(FenceHandle fence)
    {
        wait_for_fence_api(fence);
    }

    void RenderDevice::wait_queue_idle(CommandQueueType queue_type)
    {
        wait_queue_idle_api(queue_type);
    }

    void RenderDevice::wait_idle()
    {
        wait_idle_api();
    }

    void RenderDevice::bind_descriptor(const DescriptorBufferBinding& binding)
    {
        bind_descriptor_api(binding);
    }

    std::uint32_t RenderDevice::acquire_next_image(SwapchainHandle swapchain, SemaphoreHandle semaphore, FenceHandle fence)
    {
        return acquire_next_image_api(swapchain, semaphore, fence);
    }

    ImageHandle RenderDevice::get_swapchain_image(SwapchainHandle swapchain, std::uint32_t image_index)
    {
        return get_swapchain_image_api(swapchain, image_index);
    }
} // namespace orion
