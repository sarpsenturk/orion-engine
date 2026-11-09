#pragma once

#include "orion/gpu/bump_allocator.hpp"
#include "orion/gpu/deletion_queue.hpp"
#include "orion/gpu/gpu.hpp"

#include <cstddef>
#include <cstdint>

namespace orion
{
    class Frame
    {
    public:
        Frame(gpu::Device* device, std::size_t view_buffer_size);
        Frame(const Frame&)            = delete;
        Frame& operator=(const Frame&) = delete;
        Frame(Frame&&)                 = delete;
        Frame& operator=(Frame&&)      = delete;
        ~Frame();

        [[nodiscard]] std::uint64_t frame_index() const noexcept { return m_frame_index; }

    private:
        friend class Renderer;

        gpu::Device* m_device;
        std::uint64_t m_frame_index = 0ull;
        gpu::CommandAllocator* m_command_allocator;
        gpu::CommandBuffer* m_command_buffer = nullptr;
        gpu::TimelinePoint m_timeline_point;
        gpu::SwapchainFrame m_swapchain_frame;
        gpu::DeletionQueue m_deletion_queue;
        gpu::BumpAllocator m_view_allocator;
    };
} // namespace orion
