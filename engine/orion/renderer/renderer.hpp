#pragma once

#include "orion/renderer/geometry.hpp"
#include "orion/renderer/gpu/deletion_queue.hpp"
#include "orion/renderer/gpu/gpu.hpp"
#include "orion/renderer/render_graph/graph.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>

namespace orion
{
    struct RendererDesc {
        const class Window* window = nullptr;
    };

    // Valid only between begin_frame() and end_frame(). Graph handles are frame-local.
    struct RenderFrame {
        RenderGraph& graph;
        RGTexture backbuffer;
        gpu::Extent2D extent;
    };

    class Renderer
    {
    public:
        static constexpr auto frames_in_flight      = 2u;
        static constexpr auto swapchain_image_count = 3u;
        static constexpr auto swapchain_format      = gpu::Format::BGRA8_Unorm;

        explicit Renderer(const RendererDesc& desc);
        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;
        ~Renderer();

        [[nodiscard]] std::optional<RenderFrame> begin_frame();
        void end_frame();

        void resize_swapchain(std::uint32_t width, std::uint32_t height);

        [[nodiscard]] gpu::Device* device() const { return m_device.get(); }
        [[nodiscard]] GeometryManager& geometry_manager() { return m_geometry_manager; }

    private:
        struct DeviceDeleter {
            void operator()(gpu::Device* device) const { gpu::destroy_device(device); }
        };

        struct FrameData {
            gpu::CommandAllocator* command_allocator = nullptr;
            gpu::TimelinePoint timeline_point        = {};
            gpu::SwapchainFrame swapchain_frame      = {};
            DeletionQueue delete_queue;
        };

        [[nodiscard]] std::uint64_t current_frame_index() const { return m_frame_counter % frames_in_flight; }
        [[nodiscard]] FrameData& current_frame_data() { return m_frame_data[current_frame_index()]; }

        std::unique_ptr<gpu::Device, DeviceDeleter> m_device;
        gpu::Swapchain* m_swapchain;
        GeometryManager m_geometry_manager;

        std::array<FrameData, frames_in_flight> m_frame_data;
        std::uint64_t m_frame_counter = 0ull;

        RenderGraph m_render_graph;

        bool m_swapchain_out_of_date = false;
    };
} // namespace orion
