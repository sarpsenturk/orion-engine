#include "frame.hpp"

namespace orion
{
    Frame::Frame(gpu::Device* device, std::size_t view_buffer_size)
        : m_device(device)
        , m_command_allocator(gpu::create_command_allocator(m_device, gpu::Queue::Graphics))
        , m_view_allocator(view_buffer_size)
    {
    }

    Frame::~Frame()
    {
        // The renderer waits for GPU completion and flushes deletions before teardown.
        gpu::destroy_command_allocator(m_device, m_command_allocator);
    }
} // namespace orion
