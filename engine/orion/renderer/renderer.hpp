#pragma once

#include "orion/renderer/gpu.hpp"

#include <array>
#include <cstdint>

namespace orion
{
    struct RendererDesc {
        const class Window* window = nullptr;
    };

    class Renderer
    {
    public:
        static constexpr auto frames_in_flight = 2u;

        explicit Renderer(const RendererDesc& desc);
        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;
        ~Renderer();

        bool begin_frame();
        void render();
        void end_frame();

        [[nodiscard]] gpu::Device* device() const { return m_device; }

    private:
        struct FrameData {
            gpu::CommandAllocator* command_allocator = nullptr;
            gpu::TimelinePoint timeline_point        = {};
            gpu::SwapchainFrame swapchain_frame      = {};
        };

        [[nodiscard]] std::uint64_t current_frame_index() const { return m_frame_counter % frames_in_flight; }
        [[nodiscard]] FrameData& current_frame_data() { return m_frame_data[current_frame_index()]; }

        gpu::Device* m_device;
        gpu::Swapchain* m_swapchain;
        gpu::PipelineState* m_pso;

        std::array<FrameData, frames_in_flight> m_frame_data;
        std::uint64_t m_frame_counter = 0ull;
    };
} // namespace orion
