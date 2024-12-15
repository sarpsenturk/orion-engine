#include "orion/renderapi/render_device.hpp"

#include "orion/assertion.hpp"

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

    BindGroupLayoutHandle RenderDevice::create_bind_group_layout(const BindGroupLayoutDesc& desc)
    {
        auto descriptor_set_layout = create_bind_group_layout_api(desc);
        SPDLOG_DEBUG("Created bind group layout {}", fmt::underlying(descriptor_set_layout));
        return descriptor_set_layout;
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
        SPDLOG_DEBUG("Created buffer {} {{ size: {} }}", fmt::underlying(buffer), desc.size);
        return buffer;
    }

    SemaphoreHandle RenderDevice::create_semaphore(const SemaphoreDesc& desc)
    {
        auto semaphore = create_semaphore_api(desc);
        SPDLOG_DEBUG("Created semaphore {}", fmt::underlying(semaphore));
        return semaphore;
    }

    FenceHandle RenderDevice::create_fence(const FenceDesc& desc)
    {
        auto fence = create_fence_api(desc);
        SPDLOG_DEBUG("Created fence {} {{ signaled: {} }}", fmt::underlying(fence), desc.signaled);
        return fence;
    }

    BindGroupHandle RenderDevice::create_bind_group(const BindGroupDesc& desc)
    {
        auto bind_group = create_bind_group_api(desc);
        SPDLOG_DEBUG("Created bind group {}", fmt::underlying(bind_group));
        return bind_group;
    }

    ImageHandle RenderDevice::create_image(const ImageDesc& desc)
    {
        auto image = create_image_api(desc);
        SPDLOG_DEBUG("Created image {}", fmt::underlying(image));
        return image;
    }

    ImageViewHandle RenderDevice::create_image_view(const ImageViewDesc& desc)
    {
        auto image_view = create_image_view_api(desc);
        SPDLOG_DEBUG("Created image view {}", fmt::underlying(image_view));
        return image_view;
    }

    SamplerHandle RenderDevice::create_sampler(const SamplerDesc& desc)
    {
        auto sampler = create_sampler_api(desc);
        SPDLOG_DEBUG("Created sampler {}", fmt::underlying(sampler));
        return sampler;
    }

    void RenderDevice::destroy(PipelineLayoutHandle pipeline_layout)
    {
        destroy_api(pipeline_layout);
        SPDLOG_DEBUG("Destroyed pipeline layout {}", fmt::underlying(pipeline_layout));
    }

    void RenderDevice::destroy(PipelineHandle pipeline)
    {
        destroy_api(pipeline);
        SPDLOG_DEBUG("Destroyed pipeline {}", fmt::underlying(pipeline));
    }

    void RenderDevice::destroy(BufferHandle buffer)
    {
        destroy_api(buffer);
        SPDLOG_DEBUG("Destroyed buffer {}", fmt::underlying(buffer));
    }

    void RenderDevice::destroy(SemaphoreHandle semaphore)
    {
        destroy_api(semaphore);
        SPDLOG_DEBUG("Destroyed semaphore {}", fmt::underlying(semaphore));
    }

    void RenderDevice::destroy(FenceHandle fence)
    {
        destroy_api(fence);
        SPDLOG_DEBUG("Destroyed fence {}", fmt::underlying(fence));
    }

    void RenderDevice::destroy(BindGroupLayoutHandle bind_group_layout)
    {
        destroy_api(bind_group_layout);
        SPDLOG_DEBUG("Destroyed bind group layout {}", fmt::underlying(bind_group_layout));
    }

    void RenderDevice::destroy(BindGroupHandle bind_group)
    {
        destroy_api(bind_group);
        SPDLOG_DEBUG("Destroyed bind group {}", fmt::underlying(bind_group));
    }

    void RenderDevice::destroy(ImageHandle image)
    {
        destroy_api(image);
        SPDLOG_DEBUG("Destroyed image {}", fmt::underlying(image));
    }

    void RenderDevice::destroy(ImageViewHandle image_view)
    {
        destroy_api(image_view);
        SPDLOG_DEBUG("Destroyed image view {}", fmt::underlying(image_view));
    }

    void RenderDevice::destroy(SamplerHandle sampler)
    {
        destroy_api(sampler);
        SPDLOG_DEBUG("Destroyed sampler {}", fmt::underlying(sampler));
    }

    void* RenderDevice::map(BufferHandle buffer)
    {
        return map_api(buffer);
    }

    void RenderDevice::unmap(BufferHandle buffer)
    {
        unmap_api(buffer);
    }

    void RenderDevice::memcpy(BufferHandle dst, const void* src, std::size_t size)
    {
        void* ptr = map(dst);
        std::memcpy(ptr, src, size);
        unmap_api(dst);
    }

    void RenderDevice::wait_for_fence(FenceHandle fence)
    {
        ORION_ASSERT(fence != FenceHandle::Invalid);
        wait_for_fence_api(fence);
    }
} // namespace orion
