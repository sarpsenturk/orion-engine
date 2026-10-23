#pragma once

#include "orion/gpu/gpu.hpp"

#include <array>
#include <cstdint>
#include <memory>

namespace orion
{
    class Renderer
    {
    public:
        static constexpr auto frames_in_flight = 2u;

        Renderer();
        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = default;
        Renderer& operator=(Renderer&&)      = default;
        ~Renderer();

        [[nodiscard]] gpu::Device* device() const { return m_device.get(); }

    private:
        struct FrameInFlight {
            gpu::CommandAllocator* command_allocator = nullptr;
        };

        [[nodiscard]] std::uint64_t current_frame_index() const { return m_frame_counter % frames_in_flight; }
        [[nodiscard]] FrameInFlight& current_frame() { return m_frame_data[current_frame_index()]; }

        std::unique_ptr<gpu::Device, void (*)(gpu::Device*)> m_device;

        std::array<FrameInFlight, frames_in_flight> m_frame_data;
        std::uint64_t m_frame_counter = 0ull;
    };
} // namespace orion
