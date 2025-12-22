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

    RHIFence RHIDevice::create_fence(const RHIFenceDesc& desc)
    {
        return create_fence_api(desc);
    }

    RHIImageView RHIDevice::create_render_target_view(const RHIRenderTargetViewDesc& desc)
    {
        return create_render_target_view_api(desc);
    }

    RHIBuffer RHIDevice::create_buffer(const RHIBufferDesc& desc)
    {
        return create_buffer_api(desc);
    }

    void RHIDevice::destroy(RHISwapchain handle)
    {
        return destroy_api(handle);
    }

    void RHIDevice::destroy(RHIPipeline handle)
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

    void RHIDevice::destroy(RHIBuffer handle)
    {
        return destroy_api(handle);
    }

    RHIImage RHIDevice::swapchain_get_image(RHISwapchain swapchain, std::uint32_t image_idx)
    {
        return swapchain_get_image_api(swapchain, image_idx);
    }

    std::uint32_t RHIDevice::swapchain_acquire_image(RHISwapchain swapchain)
    {
        return swapchain_acquire_image_api(swapchain);
    }

    void RHIDevice::swapchain_present(RHISwapchain swapchain)
    {
        return swapchain_present_api(swapchain);
    }

    void RHIDevice::fence_wait(RHIFence fence, std::uint64_t value, std::uint64_t timeout)
    {
        return fence_wait_api(fence, value, timeout);
    }

    void RHIDevice::fence_signal(RHIFence fence, std::uint64_t value)
    {
        return fence_signal_api(fence, value);
    }

    void RHIDevice::wait_idle()
    {
        return wait_idle_api();
    }
} // namespace orion
