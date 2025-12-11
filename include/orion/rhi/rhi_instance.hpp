#pragma once

#include "orion/rhi/rhi_device.hpp"

#include <memory>

namespace orion
{
    class RHIInstance
    {
    public:
        RHIInstance() = default;
        virtual ~RHIInstance() = default;

        std::unique_ptr<RHIDevice> create_device();

    protected:
        RHIInstance(const RHIInstance&) = default;
        RHIInstance& operator=(const RHIInstance&) = default;
        RHIInstance(RHIInstance&&) = default;
        RHIInstance& operator=(RHIInstance&&) = default;

    private:
        virtual std::unique_ptr<RHIDevice> create_device_api() = 0;
    };

    std::unique_ptr<RHIInstance> rhi_create_instance();
} // namespace orion
