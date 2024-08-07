#include "orion-renderapi/render_device.h"

#include <spdlog/spdlog.h>

namespace orion
{
    RenderDevice::RenderDevice(spdlog::logger* logger)
        : logger_(logger)
    {
    }

    std::unique_ptr<CommandQueue> RenderDevice::create_queue(CommandQueueType type)
    {
        auto queue = create_queue_api(type);
        SPDLOG_LOGGER_DEBUG(logger(), "Created command queue interface at {}", fmt::ptr(queue));
        return queue;
    }

    std::unique_ptr<CommandAllocator> RenderDevice::create_command_allocator(const CommandAllocatorDesc& desc)
    {
        auto allocator = create_command_allocator_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created command allocator interface at {}", fmt::ptr(allocator));
        return allocator;
    }

    std::unique_ptr<Swapchain> RenderDevice::create_swapchain(CommandQueue* queue, const Window& window, const SwapchainDesc& desc)
    {
        auto swapchain = create_swapchain_api(queue, window, desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created swapchain with handle {}", fmt::ptr(swapchain));
        return swapchain;
    }

    std::unique_ptr<ShaderReflector> RenderDevice::create_shader_reflector()
    {
        return create_shader_reflector_api();
    }

    std::unique_ptr<RenderPass> RenderDevice::create_render_pass()
    {
        auto render_pass = create_render_pass_api();
        SPDLOG_LOGGER_DEBUG(logger(), "Created render pass interface at {}", fmt::ptr(render_pass));
        return render_pass;
    }

    ShaderModuleHandle RenderDevice::create_shader_module(const ShaderModuleDesc& desc)
    {
        auto handle = create_shader_module_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created shader module with handle {}", handle);
        return handle;
    }

    DescriptorLayoutHandle RenderDevice::create_descriptor_layout(const DescriptorLayoutDesc& desc)
    {
        auto handle = create_descriptor_layout_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created descriptor layout with handle {}", handle);
        return handle;
    }

    DescriptorPoolHandle RenderDevice::create_descriptor_pool(const DescriptorPoolDesc& desc)
    {
        auto handle = create_descriptor_pool_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created descriptor pool with handle {}", handle);
        return handle;
    }

    DescriptorHandle RenderDevice::create_descriptor(DescriptorLayoutHandle descriptor_layout_handle, DescriptorPoolHandle descriptor_pool_handle)
    {
        auto handle = create_descriptor_api(descriptor_layout_handle, descriptor_pool_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Created descriptor with handle {}", handle);
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

    FenceHandle RenderDevice::create_fence(const FenceDesc& desc)
    {
        auto handle = create_fence_api(desc);
        SPDLOG_LOGGER_DEBUG(logger(), "Created fence {}", handle);
        return handle;
    }

    SemaphoreHandle RenderDevice::create_semaphore()
    {
        auto handle = create_semaphore_api();
        SPDLOG_LOGGER_DEBUG(logger(), "Created semaphore {}", handle);
        return handle;
    }

    void RenderDevice::destroy(ShaderModuleHandle shader_module_handle)
    {
        destroy_api(shader_module_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed shader module {}", shader_module_handle);
    }

    void RenderDevice::destroy(DescriptorLayoutHandle descriptor_layout_handle)
    {
        destroy_api(descriptor_layout_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed descriptor layout {}", descriptor_layout_handle);
    }

    void RenderDevice::destroy(DescriptorPoolHandle descriptor_pool_handle)
    {
        destroy_api(descriptor_pool_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed descriptor pool {}", descriptor_pool_handle);
    }

    void RenderDevice::destroy(DescriptorHandle descriptor_handle)
    {
        destroy_api(descriptor_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed descriptor {}", descriptor_handle);
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

    void RenderDevice::destroy(FenceHandle fence_handle)
    {
        destroy_api(fence_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed fence {}", fence_handle);
    }

    void RenderDevice::destroy(SemaphoreHandle semaphore_handle)
    {
        destroy_api(semaphore_handle);
        SPDLOG_LOGGER_DEBUG(logger(), "Destroyed semaphore {}", semaphore_handle);
    }

    void RenderDevice::destroy_flush()
    {
        destroy_flush_api();
    }

    void* RenderDevice::map(GPUBufferHandle buffer_handle)
    {
        return map_api(buffer_handle);
    }

    void* RenderDevice::map(ImageHandle image_handle)
    {
        return map_api(image_handle);
    }

    void RenderDevice::unmap(GPUBufferHandle buffer_handle)
    {
        return unmap_api(buffer_handle);
    }

    void RenderDevice::unmap(ImageHandle image_handle)
    {
        return unmap_api(image_handle);
    }

    void RenderDevice::wait_for_fence(FenceHandle fence_handle)
    {
        wait_for_fences_api({&fence_handle, 1});
    }

    void RenderDevice::wait_for_fences(std::span<const FenceHandle> fence_handles)
    {
        wait_for_fences_api(fence_handles);
    }

    void RenderDevice::wait_queue_idle(CommandQueueType queue_type)
    {
        wait_queue_idle_api(queue_type);
    }

    void RenderDevice::wait_idle()
    {
        wait_idle_api();
    }

    void RenderDevice::reset_descriptor_pool(DescriptorPoolHandle descriptor_pool_handle)
    {
        reset_descriptor_pool_api(descriptor_pool_handle);
    }

    void RenderDevice::write_descriptor(DescriptorHandle descriptor_handle, std::span<const DescriptorWrite> writes)
    {
        write_descriptor_api(descriptor_handle, writes);
    }

    void RenderDevice::write_descriptor(DescriptorHandle descriptor_handle, const DescriptorWrite& write)
    {
        write_descriptor_api(descriptor_handle, {{write}});
    }

    void RenderDevice::write_descriptor(DescriptorHandle descriptor_handle, std::uint32_t binding, DescriptorType type, const BufferDescriptorDesc& buffer)
    {
        const auto write = DescriptorWrite{
            .binding = binding,
            .descriptor_type = type,
            .array_start = 0,
            .buffers = {&buffer, 1},
        };
        write_descriptor(descriptor_handle, write);
    }

    void RenderDevice::write_descriptor(DescriptorHandle descriptor_handle, std::uint32_t binding, ImageViewHandle image_view_handle, ImageLayout layout)
    {
        const auto image_write = ImageDescriptorDesc{
            .image_view_handle = image_view_handle,
            .image_layout = layout,
        };
        const auto write = DescriptorWrite{
            .binding = binding,
            .descriptor_type = DescriptorType::SampledImage,
            .array_start = 0,
            .images = {&image_write, 1},
        };
        write_descriptor(descriptor_handle, write);
    }

    void RenderDevice::write_descriptor(DescriptorHandle descriptor_handle, std::uint32_t binding, SamplerHandle sampler)
    {
        const auto sampler_write = ImageDescriptorDesc{
            .sampler_handle = sampler,
        };
        const auto write = DescriptorWrite{
            .binding = binding,
            .descriptor_type = DescriptorType::Sampler,
            .array_start = 0,
            .images = {&sampler_write, 1},
        };
        write_descriptor(descriptor_handle, write);
    }
} // namespace orion
