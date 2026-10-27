#pragma once

#include "mesh.hpp"
#include "scene.hpp"

#include "orion/core/event.hpp"
#include "orion/gpu/gpu.hpp"

#include <glm/mat4x4.hpp>

#include <array>
#include <cstdint>
#include <flat_map>
#include <memory>
#include <optional>
#include <vector>

namespace orion
{
    // Forward declarations
    class Window;

    struct RenderView {
        glm::mat4 view;
        glm::mat4 projection;
        const Scene& scene;
    };

    struct Frame {
    };

    struct RendererDesc {
        std::uint32_t width;
        std::uint32_t height;
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

        std::optional<Frame> begin_frame(gpu::Swapchain* swapchain);
        void render(const RenderView& view);

        [[nodiscard]] gpu::Swapchain* create_swapchain(
            const Window& window,
            std::uint32_t image_count,
            gpu::Format image_format);
        void on_event(const Event& event);

        [[nodiscard]] Mesh create_mesh(const MeshDesc& desc);
        void destroy_mesh(Mesh mesh);

        [[nodiscard]] gpu::Device* device() const { return m_device.get(); }

    private:
        struct FrameInFlight {
            gpu::CommandAllocator* command_allocator = nullptr;
            gpu::TimelinePoint timeline_point;
            gpu::SwapchainFrame swapchain_frame;
        };

        [[nodiscard]] std::uint64_t current_frame_index() const { return m_frame_counter % frames_in_flight; }
        [[nodiscard]] FrameInFlight& current_frame() { return m_frame_data[current_frame_index()]; }

        std::unique_ptr<gpu::Device, void (*)(gpu::Device*)> m_device;
        gpu::Extent2D m_renderer_extent;
        gpu::Image* m_depth_image          = nullptr;
        gpu::ImageView* m_depth_image_view = nullptr;

        std::flat_map<const Window*, gpu::Swapchain*> m_swapchains;

        std::array<FrameInFlight, frames_in_flight> m_frame_data;
        std::uint64_t m_frame_counter = 0ull;

        MeshManager m_mesh_manager;

        gpu::PipelineState* m_dummy_pipeline = nullptr;
    };
} // namespace orion
