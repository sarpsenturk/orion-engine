#include "orion/renderapi/render_device.h"

#include "orion/assertion.h"

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

    DescriptorSetLayoutHandle RenderDevice::create_descriptor_set_layout(const DescriptorSetLayoutDesc& desc)
    {
        auto descriptor_set_layout = create_descriptor_set_layout_api(desc);
        SPDLOG_DEBUG("Created descriptor set layout {}", fmt::underlying(descriptor_set_layout));
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

    DescriptorPoolHandle RenderDevice::create_descriptor_pool(const DescriptorPoolDesc& desc)
    {
        auto descriptor_pool = create_descriptor_pool_api(desc);
        SPDLOG_DEBUG("Created descriptor pool {}", fmt::underlying(descriptor_pool));
        return descriptor_pool;
    }

    DescriptorSetHandle RenderDevice::create_descriptor_set(const DescriptorSetDesc& desc)
    {
        auto descriptor_set = create_descriptor_set_api(desc);
        SPDLOG_DEBUG("Created descriptor set {}", fmt::underlying(descriptor_set));
        return descriptor_set;
    }

    ImageHandle RenderDevice::create_image(const ImageDesc& desc)
    {
        auto image = create_image_api(desc);
        SPDLOG_DEBUG("Created image {}", fmt::underlying(image));
        return image;
    }

    void RenderDevice::create_constant_buffer_view(const ConstantBufferViewDesc& desc)
    {
        ORION_ASSERT(desc.buffer != BufferHandle::Invalid);
        ORION_ASSERT(desc.descriptor_set != DescriptorSetHandle::Invalid);
        create_constant_buffer_view_api(desc);
    }

    ImageViewHandle RenderDevice::create_image_view(const ImageViewDesc& desc)
    {
        ORION_ASSERT(desc.format != Format::Undefined);
        return create_image_view_api(desc);
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

    void RenderDevice::destroy(DescriptorPoolHandle descriptor_pool)
    {
        destroy_api(descriptor_pool);
        SPDLOG_DEBUG("Destroyed descriptor pool {}", fmt::underlying(descriptor_pool));
    }

    void RenderDevice::destroy(DescriptorSetLayoutHandle descriptor_set_layout)
    {
        destroy_api(descriptor_set_layout);
        SPDLOG_DEBUG("Destroyed descriptor set_layout {}", fmt::underlying(descriptor_set_layout));
    }

    void RenderDevice::destroy(DescriptorSetHandle descriptor_set)
    {
        destroy_api(descriptor_set);
        SPDLOG_DEBUG("Destroyed descriptor set {}", fmt::underlying(descriptor_set));
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
