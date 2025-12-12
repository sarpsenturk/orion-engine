#include "orion/rhi/device.hpp"

namespace orion
{
    std::unique_ptr<RHICommandQueue> RHIDevice::create_command_queue(const RHICommandQueueDesc& desc)
    {
        return create_command_queue_api(desc);
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
} // namespace orion
