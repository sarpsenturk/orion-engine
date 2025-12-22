#pragma once

#include "orion/rhi/buffer.hpp"
#include "orion/rhi/command.hpp"
#include "orion/rhi/handle.hpp"
#include "orion/rhi/image.hpp"
#include "orion/rhi/pipeline.hpp"
#include "orion/rhi/swapchain.hpp"
#include "orion/rhi/synchronization.hpp"

#include <memory>

namespace orion
{
    class RHIDevice
    {
    public:
        RHIDevice() = default;
        virtual ~RHIDevice() = default;

        std::unique_ptr<RHICommandQueue> create_command_queue(const RHICommandQueueDesc& desc);
        std::unique_ptr<RHICommandAllocator> create_command_allocator(const RHICommandAllocatorDesc& desc);
        std::unique_ptr<RHICommandList> create_command_list(const RHICommandListDesc& desc);

        RHISwapchain create_swapchain(const RHISwapchainDesc& desc);
        RHIPipeline create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc);
        RHIFence create_fence(const RHIFenceDesc& desc);
        RHIImageView create_render_target_view(const RHIRenderTargetViewDesc& desc);
        RHIBuffer create_buffer(const RHIBufferDesc& desc);

        void destroy(RHISwapchain handle);
        void destroy(RHIPipeline handle);
        void destroy(RHIFence handle);
        void destroy(RHIImageView handle);
        void destroy(RHIBuffer handle);

        RHIImage swapchain_get_image(RHISwapchain swapchain, std::uint32_t image_idx);
        std::uint32_t swapchain_acquire_image(RHISwapchain swapchain);
        void swapchain_present(RHISwapchain swapchain);

        void fence_wait(RHIFence fence, std::uint64_t value, std::uint64_t timeout);
        void fence_signal(RHIFence fence, std::uint64_t value);

        void wait_idle();

    protected:
        RHIDevice(const RHIDevice&) = default;
        RHIDevice& operator=(const RHIDevice&) = default;
        RHIDevice(RHIDevice&&) = default;
        RHIDevice& operator=(RHIDevice&&) = default;

    private:
        virtual std::unique_ptr<RHICommandQueue> create_command_queue_api(const RHICommandQueueDesc& desc) = 0;
        virtual std::unique_ptr<RHICommandAllocator> create_command_allocator_api(const RHICommandAllocatorDesc& desc) = 0;
        virtual std::unique_ptr<RHICommandList> create_command_list_api(const RHICommandListDesc& desc) = 0;

        virtual RHISwapchain create_swapchain_api(const RHISwapchainDesc& desc) = 0;
        virtual RHIPipeline create_graphics_pipeline_api(const RHIGraphicsPipelineDesc& desc) = 0;
        virtual RHIFence create_fence_api(const RHIFenceDesc& desc) = 0;
        virtual RHIImageView create_render_target_view_api(const RHIRenderTargetViewDesc& desc) = 0;
        virtual RHIBuffer create_buffer_api(const RHIBufferDesc& desc) = 0;

        virtual void destroy_api(RHISwapchain handle) = 0;
        virtual void destroy_api(RHIPipeline handle) = 0;
        virtual void destroy_api(RHIFence handle) = 0;
        virtual void destroy_api(RHIImageView handle) = 0;
        virtual void destroy_api(RHIBuffer handle) = 0;

        virtual RHIImage swapchain_get_image_api(RHISwapchain swapchain, std::uint32_t image_idx) = 0;
        virtual std::uint32_t swapchain_acquire_image_api(RHISwapchain swapchain) = 0;
        virtual void swapchain_present_api(RHISwapchain swapchain) = 0;

        virtual void fence_wait_api(RHIFence fence, std::uint64_t value, std::uint64_t timeout) = 0;
        virtual void fence_signal_api(RHIFence fence, std::uint64_t value) = 0;

        virtual void wait_idle_api() = 0;
    };
} // namespace orion
