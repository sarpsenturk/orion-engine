#pragma once

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
        RHISemaphore create_semaphore(const RHISemaphoreDesc& desc);
        RHIFence create_fence(const RHIFenceDesc& desc);
        RHIImageView create_render_target_view(const RHIRenderTargetViewDesc& desc);

        void destroy(RHISwapchain handle);
        void destroy(RHIPipeline handle);
        void destroy(RHISemaphore handle);
        void destroy(RHIFence handle);
        void destroy(RHIImageView handle);

        RHIImage get_swapchain_image(RHISwapchain swapchain, std::uint32_t image_idx);
        std::uint32_t acquire_swapchain_image(RHISwapchain swapchain, RHISemaphore semaphore, RHIFence fence);
        void swapchain_present(RHISwapchain swapchain, std::span<const RHISemaphore> wait_semaphores);

        bool wait_for_fences(std::span<const RHIFence> fences, bool wait_all, std::uint64_t timeout);
        bool reset_fences(std::span<const RHIFence> fences);

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
        virtual RHISemaphore create_semaphore_api(const RHISemaphoreDesc& desc) = 0;
        virtual RHIFence create_fence_api(const RHIFenceDesc& desc) = 0;
        virtual RHIImageView create_render_target_view_api(const RHIRenderTargetViewDesc& desc) = 0;

        virtual void destroy_api(RHISwapchain handle) = 0;
        virtual void destroy_api(RHIPipeline handle) = 0;
        virtual void destroy_api(RHISemaphore handle) = 0;
        virtual void destroy_api(RHIFence handle) = 0;
        virtual void destroy_api(RHIImageView handle) = 0;

        virtual RHIImage get_swapchain_image_api(RHISwapchain swapchain, std::uint32_t image_idx) = 0;
        virtual std::uint32_t acquire_swapchain_image_api(RHISwapchain swapchain, RHISemaphore semaphore, RHIFence fence) = 0;
        virtual void swapchain_present_api(RHISwapchain swapchain, std::span<const RHISemaphore> wait_semaphores) = 0;

        virtual bool wait_for_fences_api(std::span<const RHIFence> fences, bool wait_all, std::uint64_t timeout) = 0;
        virtual bool reset_fences_api(std::span<const RHIFence> fences) = 0;
    };
} // namespace orion
