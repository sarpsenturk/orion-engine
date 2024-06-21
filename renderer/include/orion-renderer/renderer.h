#pragma once

#include "orion-renderer/buffer.h"
#include "orion-renderer/camera.h"
#include "orion-renderer/frame.h"
#include "orion-renderer/imgui.h"
#include "orion-renderer/material.h"
#include "orion-renderer/mesh.h"
#include "orion-renderer/render_context.h"
#include "orion-renderer/shader.h"
#include "orion-renderer/texture.h"
#include "orion-renderer/types.h"

#include "orion-core/module.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_command.h"
#include "orion-renderapi/render_device.h"

#include "orion-math/matrix/matrix4.h"
#include "orion-math/vector/vector2.h"

#include "orion-core/filesystem.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace orion
{
    struct RendererDesc {
        const char* backend = nullptr;
        Vector2_u render_size;
    };

    struct RenderObj {
        mesh_id_t mesh;
        material_id_t material;
        const Matrix4_f* transform;
    };

    inline constexpr std::uint32_t descriptor_index_frame = 0;
    inline constexpr std::uint32_t descriptor_index_material = 1;
    inline constexpr std::uint32_t descriptor_index_object = 2;

    namespace textures
    {
        inline constexpr texture_id_t white = 0;
    }

    namespace meshes
    {
        inline constexpr mesh_id_t quad = 0;
    }

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        void draw(const RenderObj& obj);
        void draw_quad(material_id_t material, const Matrix4_f& transform);
        void render(const Camera& camera);
        void present_to(Swapchain* swapchain);

        template<typename ImGuiLayerT>
        std::unique_ptr<ImGuiLayerT> create_imgui_layer()
        {
            imgui_init();
            auto layer = std::make_unique<ImGuiLayerT>();
            imgui_ = layer.get();
            return layer;
        }

        std::unique_ptr<Swapchain> create_swapchain(const Window& window);

        ShaderEffect create_shader_effect(const FilePath& vs_path, const FilePath& ps_path);

        std::pair<mesh_id_t, Mesh*> create_mesh(std::span<const Vertex> vertices, std::span<const vertex_index_t> indices);
        std::pair<material_id_t, Material*> create_material(const MaterialData& data);
        std::pair<texture_id_t, Texture*> create_texture(TextureInfo info, std::span<const std::byte> bytes);

        [[nodiscard]] Mesh* find_mesh(mesh_id_t mesh_id);
        [[nodiscard]] Texture* find_texture(texture_id_t texture_id);
        [[nodiscard]] Material* find_material(material_id_t material_id);

        [[nodiscard]] std::int32_t frame_count() const { return frame_counter_; }

    private:
        struct FrameData {
            std::unique_ptr<CommandAllocator> command_allocator;

            std::unique_ptr<CommandList> render_command;
            std::unique_ptr<CommandList> present_command;

            UniqueImage render_image;
            UniqueImageView render_target;
            UniqueDescriptor render_output_descriptor;

            UniqueFence frame_fence;
            UniqueSemaphore render_semaphore;
            UniqueSemaphore swapchain_image_semaphore;

            UniqueGPUBuffer staging_buffer;
        };

        FrameData create_frame_data(frame_index_t frame_index);

        void imgui_init();
        void create_default_textures();
        void create_default_meshes();

        static constexpr std::size_t max_render_objects = 1000;
        static constexpr std::size_t staging_buffer_size = 1024 * 1024;

        Module render_backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
        std::unique_ptr<CommandQueue> render_queue_;
        Vector2_u render_size_;

        ShaderEffect object_effect_;
        ShaderEffect present_effect_;

        Pipeline object_pipeline_;
        Pipeline present_pipeline_;

        std::unique_ptr<RenderPass> color_pass_;
        std::unique_ptr<RenderPass> present_pass_;

        DescriptorPoolHandle descriptor_pool_;

        PerFrame<FrameData> frame_data_;
        std::int32_t frame_counter_ = 0;

        FrameData& current_frame() { return frame_data_[frame_counter_ % frames_in_flight]; }
        FrameData& previous_frame() { return frame_data_[(frame_counter_ - 1) % frames_in_flight]; }

        TransferContext transfer_context();

        PerFrame<UniqueDescriptor> scene_descriptors_;
        PerFrame<UniqueDescriptor> object_data_descriptors_;

        DynamicGPUBuffer scene_cbuffer_;
        DynamicGPUBuffer object_buffer_;

        SamplerHandle present_sampler_;

        std::vector<std::pair<mesh_id_t, Mesh>> meshes_;
        std::vector<std::pair<material_id_t, Material>> materials_;
        std::vector<std::pair<texture_id_t, Texture>> textures_;

        std::vector<RenderObj> objects_;

        ImGuiLayer* imgui_;
    };
} // namespace orion
