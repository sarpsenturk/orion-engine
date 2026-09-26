#pragma once

#include "orion/renderer/gpu.hpp"
#include "orion/renderer/mesh.hpp"

#include "orion/lib/slot_map.hpp"

#include <array>
#include <cstdint>
#include <span>

namespace orion
{
    using MeshHandle = typename SlotMap<Mesh>::Handle;

    struct RendererDesc {
        const class Window* window = nullptr;
    };

    class Renderer
    {
    public:
        static constexpr auto frames_in_flight = 2u;
        static constexpr auto max_vertex_count = 1'024u;
        static constexpr auto max_index_count  = 4'096u;

        explicit Renderer(const RendererDesc& desc);
        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;
        ~Renderer();

        bool begin_frame();
        void render();
        void end_frame();

        MeshHandle create_mesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices);
        void destroy_mesh(MeshHandle mesh);

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

        gpu::Buffer* m_vertex_buffer;
        gpu::Buffer* m_index_buffer;
        std::int32_t m_vertex_offset = 0u;
        std::uint32_t m_index_offset = 0u;
        SlotMap<Mesh> m_meshes;

        std::array<FrameData, frames_in_flight> m_frame_data;
        std::uint64_t m_frame_counter = 0ull;
    };
} // namespace orion
