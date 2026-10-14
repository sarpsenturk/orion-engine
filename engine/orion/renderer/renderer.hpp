#pragma once

#include "orion/renderer/camera.hpp"
#include "orion/renderer/gpu/deletion_queue.hpp"
#include "orion/renderer/gpu/gpu.hpp"

#include <glm/mat4x4.hpp>

#include <imgui.h>

#include <array>
#include <cstdint>
#include <span>

namespace orion
{
    struct View {
        Camera camera;
    };

    struct RendererDesc {
        const class Window* window = nullptr;
    };

    class Renderer
    {
    public:
        static constexpr auto frames_in_flight   = 2u;
        static constexpr auto max_vertex_count   = 1'024u;
        static constexpr auto max_index_count    = 4'096u;
        static constexpr auto max_material_count = 128u;

        explicit Renderer(const RendererDesc& desc);
        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;
        ~Renderer();

        bool begin_frame();
        void render(const View& view);
        void end_frame();

        void resize_swapchain(std::uint32_t width, std::uint32_t height);

        [[nodiscard]] gpu::Device* device() const { return m_device; }

    private:
        struct FrameData {
            gpu::CommandAllocator* command_allocator = nullptr;
            gpu::TimelinePoint timeline_point        = {};
            gpu::SwapchainFrame swapchain_frame      = {};
            DeletionQueue delete_queue;
        };

        [[nodiscard]] std::uint64_t current_frame_index() const { return m_frame_counter % frames_in_flight; }
        [[nodiscard]] FrameData& current_frame_data() { return m_frame_data[current_frame_index()]; }

        gpu::Device* m_device;
        gpu::Swapchain* m_swapchain;

        gpu::Image* m_depth_image;
        gpu::ImageView* m_depth_image_view;

        std::array<FrameData, frames_in_flight> m_frame_data;
        std::uint64_t m_frame_counter = 0ull;

        bool m_swapchain_out_of_date = false;
    };
} // namespace orion
