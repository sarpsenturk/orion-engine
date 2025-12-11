#pragma once

#include "orion/rhi/command.hpp"
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

    protected:
        RHIDevice(const RHIDevice&) = default;
        RHIDevice& operator=(const RHIDevice&) = default;
        RHIDevice(RHIDevice&&) = default;
        RHIDevice& operator=(RHIDevice&&) = default;

    private:
        virtual std::unique_ptr<RHICommandQueue> create_command_queue_api(const RHICommandQueueDesc& desc) = 0;
        virtual std::unique_ptr<RHISwapchain> create_swapchain_api(const RHISwapchainDesc& desc) = 0;
    };
} // namespace orion
