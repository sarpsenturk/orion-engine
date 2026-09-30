#pragma once

#include "orion/lib/slot_map.hpp"
#include "orion/renderer/camera.hpp"
#include "orion/renderer/deletion_queue.hpp"
#include "orion/renderer/gpu.hpp"
#include "orion/renderer/material.hpp"
#include "orion/renderer/mesh.hpp"
#include "orion/renderer/texture.hpp"

#include <glm/mat4x4.hpp>

#include <imgui.h>

#include <array>
#include <cstdint>
#include <span>

namespace orion
{
    struct RenderObject {
        Mesh mesh;
        glm::mat4 model;
        Material material;
    };

    struct View {
        Camera camera;
        std::span<const RenderObject> objects;
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

        Mesh create_mesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices);
        void destroy_mesh(Mesh mesh);

        Texture create_texture(const TextureDesc& desc, std::span<const std::byte> data);
        void destroy_texture(Texture texture);

        Material create_material(const MaterialDesc& desc);
        void destroy_material(Material material);

        void resize_swapchain(std::uint32_t width, std::uint32_t height);

        [[nodiscard]] gpu::Device* device() const { return m_device; }

    private:
        struct FrameData {
            gpu::CommandAllocator* command_allocator = nullptr;
            gpu::TimelinePoint timeline_point        = {};
            gpu::SwapchainFrame swapchain_frame      = {};
            DeletionQueue delete_queue;
        };

        struct SceneData {
            glm::mat4 view_projection;
        };

        struct PushConstants {
            gpu::Address vertex_buffer_address = 0ull;
            gpu::Address scene_buffer_address  = 0ull;
            alignas(16) glm::mat4 model        = glm::mat4(1.0f);
        };
        static_assert(sizeof(PushConstants) <= gpu::max_push_constants_size);

        struct TextureData {
            gpu::Image* image          = nullptr;
            gpu::ImageView* image_view = nullptr;
        };

        struct MaterialData {
            std::uint32_t albedo_texture_idx = 0u;
            std::uint32_t sampler_idx        = 0u;
        };

        [[nodiscard]] std::uint64_t current_frame_index() const { return m_frame_counter % frames_in_flight; }
        [[nodiscard]] FrameData& current_frame_data() { return m_frame_data[current_frame_index()]; }

        void queue_delete(gpu::Image* image) { current_frame_data().delete_queue.push(image); }
        void queue_delete(gpu::ImageView* image_view) { current_frame_data().delete_queue.push(image_view); }
        void queue_delete(gpu::Buffer* buffer) { current_frame_data().delete_queue.push(buffer); }

        gpu::Device* m_device;
        gpu::Swapchain* m_swapchain;
        gpu::PipelineState* m_pso;

        gpu::Buffer* m_vertex_buffer;
        gpu::Buffer* m_index_buffer;
        std::int32_t m_vertex_offset = 0u;
        std::uint32_t m_index_offset = 0u;

        SlotMap<TextureData> m_textures;
        SlotMap<MaterialData> m_materials;

        gpu::Buffer* m_scene_buffer;
        gpu::Buffer* m_material_buffer;

        std::array<FrameData, frames_in_flight> m_frame_data;
        std::uint64_t m_frame_counter = 0ull;

        bool m_swapchain_out_of_date = false;
    };
} // namespace orion
