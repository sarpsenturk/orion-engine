#pragma once

#include "orion/rhi/handle.hpp"
#include "orion/rhi/image.hpp"

#include <span>

namespace orion
{
    enum class RHICommandQueueType {
        Graphics,
    };

    struct RHICommandAllocatorDesc {
        RHICommandQueueType type;
    };

    class RHICommandAllocator
    {
    public:
        RHICommandAllocator() = default;
        virtual ~RHICommandAllocator() = default;

        bool reset();

    protected:
        RHICommandAllocator(const RHICommandAllocator&) = default;
        RHICommandAllocator& operator=(const RHICommandAllocator&) = default;
        RHICommandAllocator(RHICommandAllocator&&) = default;
        RHICommandAllocator& operator=(RHICommandAllocator&&) = default;

    private:
        virtual bool reset_api() = 0;
    };

    struct RHICommandListDesc {
        RHICommandAllocator* command_allocator;
    };

    struct RHITransitionBarrier {
        RHIImage image;
        RHIImageLayout old_layout;
        RHIImageLayout new_layout;
    };

    struct RHICmdPipelineBarrier {
        std::span<const RHITransitionBarrier> transition_barriers;
    };

    class RHICommandList
    {
    public:
        RHICommandList() = default;
        virtual ~RHICommandList() = default;

        bool reset();

        void begin();
        void end();

        void pipeline_barrier(const RHICmdPipelineBarrier& cmd);

    protected:
        RHICommandList(const RHICommandList&) = default;
        RHICommandList& operator=(const RHICommandList&) = default;
        RHICommandList(RHICommandList&&) = default;
        RHICommandList& operator=(RHICommandList&&) = default;

    private:
        virtual bool reset_api() = 0;

        virtual void begin_api() = 0;
        virtual void end_api() = 0;

        virtual void pipeline_barrier_api(const RHICmdPipelineBarrier& cmd) = 0;
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
