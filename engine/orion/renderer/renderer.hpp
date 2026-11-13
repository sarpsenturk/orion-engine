#pragma once

#include "frame.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "scene.hpp"
#include "texture.hpp"

#include "orion/core/event.hpp"
#include "orion/gpu/deletion_queue.hpp"
#include "orion/gpu/gpu.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <array>
#include <cstdint>
#include <flat_map>
#include <memory>

// ImGui forward declarations
struct ImDrawData;

namespace orion
{
    // Forward declarations
    class Window;

    struct SceneView {
        glm::mat4 view;
        glm::mat4 projection;
        const Scene* scene;
    };

    struct FrameStats {
        std::uint64_t frame_index = 0ull;

        // This frame's buffer backing allocations, including retained unused
        // capacity. Excludes other frames in flight, CPU storage and command buffers.
        std::uint64_t allocation_bytes = 0ull;
        // Live frame allocation sizes, excluding alignment gaps.
        std::uint64_t used_bytes = 0ull;

        // CPU elapsed times, measured in milliseconds.
        double frame_wait_ms        = 0.0;
        double swapchain_acquire_ms = 0.0;
        double scene_record_ms      = 0.0;
        double ui_record_ms         = 0.0;
        double command_submit_ms    = 0.0;
        double swapchain_present_ms = 0.0;

        // Scene geometry only; excludes ImGui draws.
        std::uint32_t instance_count = 0u;
        std::uint32_t draw_calls     = 0u;
        std::uint32_t index_count    = 0u;
        std::uint32_t triangle_count = 0u;
    };

    struct RendererStats {
        // Resources currently held by managers, including pending destruction.
        std::uint32_t mesh_count     = 0u;
        std::uint32_t material_count = 0u;
        std::uint32_t texture_count  = 0u;

        // Live mesh ranges, including pending destruction. Vertex usage includes
        // padding between attribute sections, but excludes gaps between allocations.
        std::uint64_t mesh_vertex_used_bytes = 0ull;
        std::uint64_t mesh_index_used_bytes  = 0ull;

        // Manager-owned buffer/image backing allocations in bytes, including unused
        // capacity and pending destruction. Excludes CPU storage and other GPU objects.
        std::uint64_t mesh_vertex_allocation_bytes = 0ull;
        std::uint64_t mesh_index_allocation_bytes  = 0ull;
        std::uint64_t mesh_record_allocation_bytes = 0ull;
        std::uint64_t material_allocation_bytes    = 0ull;
        std::uint64_t texture_allocation_bytes     = 0ull;
    };

    class Renderer
    {
    public:
        static constexpr auto frames_in_flight       = 2u;
        static constexpr auto swapchain_image_count  = 3u;
        static constexpr auto swapchain_image_format = gpu::Format::BGRA8_Unorm;

        Renderer();
        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;
        ~Renderer();

        // Returns nullptr if acquisition fails. The frame is owned by the renderer
        // and is valid for recording until end_frame().
        Frame* begin_frame(gpu::Swapchain* swapchain);
        void render_scene(const SceneView& view);
        void render_ui(ImDrawData* imgui_draw_data);
        void end_frame();

        [[nodiscard]] gpu::Swapchain* create_swapchain(const Window& window);
        void on_event(const Event& event);

        [[nodiscard]] Mesh create_mesh(const MeshDesc& desc);
        void destroy_mesh(Mesh mesh);

        [[nodiscard]] Material create_material(const PbrMaterial& material);
        void destroy_material(Material material);

        [[nodiscard]] Texture create_texture(const TextureDesc& desc);
        void destroy_texture(Texture texture);

        [[nodiscard]] Scene* create_scene();
        void destroy_scene(Scene* scene);

        [[nodiscard]] gpu::Device* device() const { return m_device.get(); }

        // Last completed frame; zero-initialized until the first frame completes.
        [[nodiscard]] const FrameStats& frame_stats() const noexcept { return m_last_frame_stats; }

        // Current resource snapshot, independent of frame completion.
        [[nodiscard]] RendererStats renderer_stats() const noexcept;

    private:
        [[nodiscard]] std::uint64_t current_frame_index() const { return m_frame_counter % frames_in_flight; }
        [[nodiscard]] Frame& current_frame() { return m_frame_data[current_frame_index()]; }

        std::unique_ptr<gpu::Device, void (*)(gpu::Device*)> m_device;
        gpu::Image* m_depth_image          = nullptr;
        gpu::ImageView* m_depth_image_view = nullptr;

        std::flat_map<const Window*, gpu::Swapchain*> m_swapchains;

        std::array<Frame, frames_in_flight> m_frame_data;
        gpu::DeletionQueue m_pending_deletions;
        std::uint64_t m_frame_counter = 0ull;
        FrameStats m_current_frame_stats;
        FrameStats m_last_frame_stats;

        MeshManager m_mesh_manager;
        MaterialManager m_material_manager;
        TextureManager m_texture_manager;

        gpu::PipelineState* m_dummy_pipeline = nullptr;
    };
} // namespace orion
