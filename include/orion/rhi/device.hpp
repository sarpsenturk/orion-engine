#pragma once

#include "orion/rhi/command.hpp"
#include "orion/rhi/handle.hpp"
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
        std::unique_ptr<RHISwapchain> create_swapchain(const RHISwapchainDesc& desc);

        RHIPipeline create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc);
        RHISemaphore create_semaphore(const RHISemaphoreDesc& desc);
        RHIFence create_fence(const RHIFenceDesc& desc);

        void destroy(RHIPipeline handle);
        void destroy(RHISemaphore handle);
        void destroy(RHIFence handle);

    protected:
        RHIDevice(const RHIDevice&) = default;
        RHIDevice& operator=(const RHIDevice&) = default;
        RHIDevice(RHIDevice&&) = default;
        RHIDevice& operator=(RHIDevice&&) = default;

    private:
        virtual std::unique_ptr<RHICommandQueue> create_command_queue_api(const RHICommandQueueDesc& desc) = 0;
        virtual std::unique_ptr<RHISwapchain> create_swapchain_api(const RHISwapchainDesc& desc) = 0;

        virtual RHIPipeline create_graphics_pipeline_api(const RHIGraphicsPipelineDesc& desc) = 0;
        virtual RHISemaphore create_semaphore_api(const RHISemaphoreDesc& desc) = 0;
        virtual RHIFence create_fence_api(const RHIFenceDesc& desc) = 0;

        virtual void destroy_api(RHIPipeline handle) = 0;
        virtual void destroy_api(RHISemaphore handle) = 0;
        virtual void destroy_api(RHIFence handle) = 0;
    };
} // namespace orion
