#pragma once

namespace orion
{
    enum class RHICommandQueueType {
        Graphics,
    };

    struct RHICommandQueueDesc {
        RHICommandQueueType type;
    };

    class RHICommandQueue
    {
    public:
        RHICommandQueue() = default;
        virtual ~RHICommandQueue() = default;

    protected:
        RHICommandQueue(const RHICommandQueue&) = default;
        RHICommandQueue& operator=(const RHICommandQueue&) = default;
        RHICommandQueue(RHICommandQueue&&) = default;
        RHICommandQueue& operator=(RHICommandQueue&&) = default;
    };
} // namespace orion
