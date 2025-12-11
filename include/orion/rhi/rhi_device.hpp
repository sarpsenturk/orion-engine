#pragma once

namespace orion
{
    class RHIDevice
    {
    public:
        RHIDevice() = default;
        virtual ~RHIDevice() = default;

    protected:
        RHIDevice(const RHIDevice&) = default;
        RHIDevice& operator=(const RHIDevice&) = default;
        RHIDevice(RHIDevice&&) = default;
        RHIDevice& operator=(RHIDevice&&) = default;
    };
} // namespace orion
