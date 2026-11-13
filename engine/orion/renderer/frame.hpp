#pragma once

#include "orion/gpu/deletion_queue.hpp"
#include "orion/gpu/gpu.hpp"
#include "orion/gpu/page_allocator.hpp"
#include "orion/gpu/range_allocator.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace orion
{
    class FrameAllocator
    {
    public:
        struct Allocation {
            gpu::Buffer* buffer;
            std::size_t offset;
        };

        explicit FrameAllocator(gpu::Device* device);
        FrameAllocator(const FrameAllocator&)            = delete;
        FrameAllocator& operator=(const FrameAllocator&) = delete;
        FrameAllocator(FrameAllocator&&)                 = delete;
        FrameAllocator& operator=(FrameAllocator&&)      = delete;

        [[nodiscard]] Allocation allocate(std::size_t size, std::size_t alignment = 1z);

        [[nodiscard]] std::uint64_t allocation_bytes() const noexcept;
        [[nodiscard]] std::uint64_t used_bytes() const noexcept;

        void reset();

    private:
        // Default page size for linear frame allocations.
        static constexpr std::size_t allocation_page_size = 64z * 1'024z;

        struct AllocationPage {
            gpu::Buffer* buffer;
            gpu::RangeAllocator ranges;
        };

        gpu::PageAllocator m_page_allocator;
        std::vector<AllocationPage> m_allocation_pages;
    };

    class Frame
    {
    public:
        using Allocation = FrameAllocator::Allocation;

        explicit Frame(gpu::Device* device);
        Frame(const Frame&)            = delete;
        Frame& operator=(const Frame&) = delete;
        Frame(Frame&&)                 = delete;
        Frame& operator=(Frame&&)      = delete;
        ~Frame();

        [[nodiscard]] Allocation allocate(std::size_t size, std::size_t alignment = 1z)
        {
            return m_allocator.allocate(size, alignment);
        }
        template<class T>
        [[nodiscard]] Allocation allocate(std::size_t count = 1z)
        {
            return m_allocator.allocate(sizeof(T) * count, alignof(T));
        }

        [[nodiscard]] std::uint64_t allocation_bytes() const noexcept { return m_allocator.allocation_bytes(); }
        [[nodiscard]] std::uint64_t used_bytes() const noexcept { return m_allocator.used_bytes(); }

    private:
        friend class Renderer;

        // The renderer must wait for this frame's submission before resetting.
        void reset_allocations() { m_allocator.reset(); }

        gpu::Device* m_device;

        gpu::CommandAllocator* m_command_allocator;
        gpu::CommandBuffer* m_command_buffer = nullptr;
        gpu::TimelinePoint m_timeline_point;
        gpu::SwapchainFrame m_swapchain_frame;
        gpu::DeletionQueue m_deletion_queue;

        FrameAllocator m_allocator;
    };
} // namespace orion
