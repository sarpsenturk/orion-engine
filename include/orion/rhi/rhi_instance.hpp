#pragma once

namespace orion
{
    class RHIInstance
    {
    public:
        RHIInstance() = default;
        virtual ~RHIInstance() = default;

    protected:
        RHIInstance(const RHIInstance&) = default;
        RHIInstance& operator=(const RHIInstance&) = default;
        RHIInstance(RHIInstance&&) = default;
        RHIInstance& operator=(RHIInstance&&) = default;
    };

    RHIInstance* rhi_create_instance();
    void rhi_destroy_instance(RHIInstance* instance);
} // namespace orion
