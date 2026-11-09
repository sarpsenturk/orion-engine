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
    class Frame
    {
    public:
        struct Allocation {
            gpu::Buffer* buffer;
            std::size_t offset;
        };

        explicit Frame(gpu::Device* device);
        Frame(const Frame&)            = delete;
        Frame& operator=(const Frame&) = delete;
        Frame(Frame&&)                 = delete;
        Frame& operator=(Frame&&)      = delete;
        ~Frame();

        [[nodiscard]] Allocation allocate(std::size_t size, std::size_t alignment = 1z);
        template<class T>
        [[nodiscard]] Allocation allocate()
        {
            return allocate(sizeof(T), alignof(T));
        }

    private:
        friend class Renderer;

        // Default page size for linear frame allocations.
        static constexpr std::size_t allocation_page_size = 4z * 1'024z * 1'024z;

        struct AllocationPage {
            gpu::Buffer* buffer;
            gpu::RangeAllocator ranges;
        };

        // The renderer must wait for this frame's submission before resetting.
        void reset_allocations();

        gpu::Device* m_device;

        gpu::CommandAllocator* m_command_allocator;
        gpu::CommandBuffer* m_command_buffer = nullptr;
        gpu::TimelinePoint m_timeline_point;
        gpu::SwapchainFrame m_swapchain_frame;
        gpu::DeletionQueue m_deletion_queue;

        gpu::PageAllocator m_page_allocator;
        std::vector<AllocationPage> m_allocation_pages;
    };
} // namespace orion
