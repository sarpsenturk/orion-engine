#include "frame.hpp"

#include "orion/core/assert.hpp"
#include "orion/utils/scope_guard.hpp"

#include <glm/matrix.hpp>

#include <algorithm>
#include <bit>
#include <cstring>
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

    ViewIndex Frame::create_view(const ViewDesc& desc)
    {
        const auto gpu_view = GpuView{
            .view                  = desc.view,
            .projection            = desc.projection,
            .view_projection       = desc.projection * desc.view,
            .camera_world_position = glm::inverse(desc.view)[3],
        };
        const auto [buffer, offset] = allocate<GpuView>();
        std::memcpy(
            static_cast<std::byte*>(gpu::buffer_host_address(buffer)) + offset,
            &gpu_view, sizeof(gpu_view));

        const auto index = ViewIndex{static_cast<std::uint32_t>(m_views.size())};
        m_views.push_back({
            .view                  = gpu_view.view,
            .projection            = gpu_view.projection,
            .view_projection       = gpu_view.view_projection,
            .camera_world_position = gpu_view.camera_world_position,
            .viewport              = desc.viewport,
            .scissor               = desc.scissor,
            .gpu_address           = gpu::buffer_device_address(buffer) + offset,
            .opaque_draws          = {},
        });
        return index;
    }

    const RenderView& Frame::view(ViewIndex index) const
    {
        ORION_ASSERT(index.value < m_views.size());
        return m_views[index.value];
    }

    void Frame::draw_opaque(ViewIndex index, const DrawPacket& packet)
    {
        ORION_ASSERT(index.value < m_views.size());
        m_views[index.value].opaque_draws.push_back(packet);
    }

    void Frame::reset()
    {
        m_views.clear();
        m_allocator.reset();
    }
} // namespace orion
