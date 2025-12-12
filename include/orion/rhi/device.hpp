#pragma once

#include "orion/rhi/command.hpp"
#include "orion/rhi/handle.hpp"
#include "orion/rhi/pipeline.hpp"
#include "orion/rhi/swapchain.hpp"

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

        void destroy(RHIPipeline handle);

    protected:
        RHIDevice(const RHIDevice&) = default;
        RHIDevice& operator=(const RHIDevice&) = default;
        RHIDevice(RHIDevice&&) = default;
        RHIDevice& operator=(RHIDevice&&) = default;

    private:
        virtual std::unique_ptr<RHICommandQueue> create_command_queue_api(const RHICommandQueueDesc& desc) = 0;
        virtual std::unique_ptr<RHISwapchain> create_swapchain_api(const RHISwapchainDesc& desc) = 0;

        virtual RHIPipeline create_graphics_pipeline_api(const RHIGraphicsPipelineDesc& desc) = 0;

        virtual void destroy_api(RHIPipeline handle) = 0;
    };
} // namespace orion
