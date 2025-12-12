#include "orion/rhi/device.hpp"

namespace orion
{
    std::unique_ptr<RHICommandQueue> RHIDevice::create_command_queue(const RHICommandQueueDesc& desc)
    {
        return create_command_queue_api(desc);
    }

    std::unique_ptr<RHICommandAllocator> RHIDevice::create_command_allocator(const RHICommandAllocatorDesc& desc)
    {
        return create_command_allocator_api(desc);
    }

    std::unique_ptr<RHICommandList> RHIDevice::create_command_list(const RHICommandListDesc& desc)
    {
        return create_command_list_api(desc);
    }

    RHISwapchain RHIDevice::create_swapchain(const RHISwapchainDesc& desc)
    {
        return create_swapchain_api(desc);
    }

    RHIPipeline RHIDevice::create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc)
    {
        return create_graphics_pipeline_api(desc);
    }

    RHISemaphore RHIDevice::create_semaphore(const RHISemaphoreDesc& desc)
    {
        return create_semaphore_api(desc);
    }

    RHIFence RHIDevice::create_fence(const RHIFenceDesc& desc)
    {
        return create_fence_api(desc);
    }

    RHIImageView RHIDevice::create_render_target_view(const RHIRenderTargetViewDesc& desc)
    {
        return create_render_target_view_api(desc);
    }

    void RHIDevice::destroy(RHISwapchain handle)
    {
        return destroy_api(handle);
    }

    void RHIDevice::destroy(RHIPipeline handle)
    {
        return destroy_api(handle);
    }

    void RHIDevice::destroy(RHISemaphore handle)
    {
        return destroy_api(handle);
    }

    void RHIDevice::destroy(RHIFence handle)
    {
        return destroy_api(handle);
    }

    void RHIDevice::destroy(RHIImageView handle)
    {
        return destroy_api(handle);
    }

    RHIImage RHIDevice::get_swapchain_image(RHISwapchain swapchain, std::uint32_t image_idx)
    {
        return get_swapchain_image_api(swapchain, image_idx);
    }

    std::uint32_t RHIDevice::acquire_swapchain_image(RHISwapchain swapchain, RHISemaphore semaphore, RHIFence fence)
    {
        return acquire_swapchain_image_api(swapchain, semaphore, fence);
    }
} // namespace orion
