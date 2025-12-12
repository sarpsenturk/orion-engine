#include "orion/rhi/device.hpp"

namespace orion
{
    std::unique_ptr<RHICommandQueue> RHIDevice::create_command_queue(const RHICommandQueueDesc& desc)
    {
        return create_command_queue_api(desc);
    }

    std::unique_ptr<RHISwapchain> RHIDevice::create_swapchain(const RHISwapchainDesc& desc)
    {
        return create_swapchain_api(desc);
    }

    RHIPipeline RHIDevice::create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc)
    {
        return create_graphics_pipeline_api(desc);
    }

    void RHIDevice::destroy(RHIPipeline handle)
    {
        return destroy_api(handle);
    }
} // namespace orion
