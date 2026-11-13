#include "frame.hpp"

#include "orion/core/assert.hpp"
#include "orion/utils/scope_guard.hpp"

#include <algorithm>
#include <bit>
#include <limits>
#include <utility>

namespace orion
{
    FrameAllocator::FrameAllocator(gpu::Device* device)
        : m_page_allocator(device, {.size = allocation_page_size, .memory = gpu::Memory::CpuVisible})
    {
    }

    FrameAllocator::Allocation FrameAllocator::allocate(std::size_t size, std::size_t alignment)
    {
        ORION_ASSERT(size != 0z);
        ORION_ASSERT(std::has_single_bit(alignment));
        ORION_ASSERT(size <= std::numeric_limits<std::size_t>::max() - (alignment - 1z));

        for (auto& page : m_allocation_pages) {
            const auto allocation = page.ranges.allocate(size, alignment);
            if (allocation) { return {page.buffer, allocation.offset}; }
        }

        const auto page_size  = std::max(allocation_page_size, size);
        auto* buffer          = m_page_allocator.acquire(page_size);
        const auto guard      = ScopeFail{[&] { m_page_allocator.release(buffer); }};
        auto ranges           = gpu::RangeAllocator{page_size, gpu::RangeAllocator::Algorithm::Linear};
        const auto allocation = ranges.allocate(size, alignment);
        ORION_ASSERT(allocation);
        m_allocation_pages.push_back({buffer, std::move(ranges)});
        return {buffer, allocation.offset};
    }

    std::uint64_t FrameAllocator::allocation_bytes() const noexcept
    {
        auto bytes = std::uint64_t{0};
        for (const auto& page : m_allocation_pages) { bytes += gpu::buffer_allocation_size(page.buffer); }
        return bytes;
    }

    std::uint64_t FrameAllocator::used_bytes() const noexcept
    {
        auto bytes = std::uint64_t{0};
        for (const auto& page : m_allocation_pages) { bytes += page.ranges.used_bytes(); }
        return bytes;
    }

    void FrameAllocator::reset()
    {
        for (auto& page : m_allocation_pages) { page.ranges.reset(); }
    }

    Frame::Frame(gpu::Device* device)
        : m_device(device)
        , m_command_allocator(gpu::create_command_allocator(m_device, gpu::Queue::Graphics))
        , m_allocator(device)
    {
    }

    Frame::~Frame()
    {
        gpu::destroy_command_allocator(m_device, m_command_allocator);
    }
} // namespace orion
