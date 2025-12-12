#pragma once

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

    class RHICommandList
    {
    public:
        RHICommandList() = default;
        virtual ~RHICommandList() = default;

        bool reset();

    protected:
        RHICommandList(const RHICommandList&) = default;
        RHICommandList& operator=(const RHICommandList&) = default;
        RHICommandList(RHICommandList&&) = default;
        RHICommandList& operator=(RHICommandList&&) = default;

    private:
        virtual bool reset_api() = 0;
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
