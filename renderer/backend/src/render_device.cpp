#include "orion-renderapi/render_device.h"

#include <spdlog/spdlog.h>

namespace orion
{
    DeviceResourceControlBlock::DeviceResourceControlBlock(RenderDevice* device)
        : device_(device)
    {
    }

    std::size_t DeviceResourceControlBlock::add_ref()
    {
        return ref_count_.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    std::size_t DeviceResourceControlBlock::remove_ref()
    {
        return ref_count_.fetch_sub(1, std::memory_order_acq_rel) - 1;
    }

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

    void RenderDevice::recreate(SwapchainHandle swapchain_handle, const SwapchainDesc& desc)
    {
        recreate_api(swapchain_handle, desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Recreated swapchain with handle {}", swapchain_handle);
    }

    void RenderDevice::recreate(FramebufferHandle framebuffer_handle, const FramebufferDesc& desc)
    {
        recreate_api(framebuffer_handle, desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Recreated framebuffer with handle {}", framebuffer_handle);
    }

    void RenderDevice::destroy(SwapchainHandle swapchain_handle)
    {
        destroy_api(swapchain_handle);
    }

    void RenderDevice::destroy(RenderPassHandle render_pass_handle)
    {
        destroy_api(render_pass_handle);
    }

    void RenderDevice::destroy(FramebufferHandle framebuffer_handle)
    {
        destroy_api(framebuffer_handle);
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

    void RenderDevice::destroy(DescriptorPoolHandle descriptor_pool_handle)
    {
        destroy_api(descriptor_pool_handle);
    }

    void RenderDevice::destroy(DescriptorSetHandle descriptor_set_handle)
    {
        destroy_api(descriptor_set_handle);
    }

    void RenderDevice::destroy(SemaphoreHandle semaphore_handle)
    {
        destroy_api(semaphore_handle);
    }

    void RenderDevice::destroy(FenceHandle fence_handle)
    {
        destroy_api(fence_handle);
    }

    void RenderDevice::destroy(ImageHandle image_handle)
    {
        destroy_api(image_handle);
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

    void RenderDevice::submit_immediate(const SubmitDesc& desc)
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

    void RenderDevice::update_descriptors(const DescriptorUpdate& update)
    {
        update_descriptors_api(update);
    }

    std::uint32_t RenderDevice::acquire_next_image(SwapchainHandle swapchain, SemaphoreHandle semaphore, FenceHandle fence)
    {
        return acquire_next_image_api(swapchain, semaphore, fence);
    }
} // namespace orion
