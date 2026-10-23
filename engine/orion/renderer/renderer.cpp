#include "renderer.hpp"

#include "orion/utils/array.hpp"

namespace orion
{
    Renderer::Renderer()
        : m_device(gpu::create_device(gpu::DeviceDesc{}), gpu::destroy_device)
        , m_frame_data(make_array<frames_in_flight>([this] {
            return FrameInFlight{
                .command_allocator = gpu::create_command_allocator(m_device.get(), gpu::Queue::Graphics),
            };
        }))
    {
    }

    Renderer::~Renderer()
    {
        for (auto& frame : m_frame_data) {
            gpu::destroy_command_allocator(m_device.get(), frame.command_allocator);
        }
    }
} // namespace orion
