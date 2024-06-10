#pragma once

#include "orion-renderer/buffer.h"
#include "orion-renderer/camera.h"
#include "orion-renderer/config.h"
#include "orion-renderer/imgui.h"
#include "orion-renderer/material.h"
#include "orion-renderer/mesh.h"
#include "orion-renderer/render_context.h"
#include "orion-renderer/render_target.h"
#include "orion-renderer/render_window.h"
#include "orion-renderer/shader.h"
#include "orion-renderer/texture.h"
#include "orion-renderer/types.h"

#include "orion-core/module.h"
#include "orion-platform/platform.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_command.h"
#include "orion-renderapi/render_device.h"
#include "orion-renderapi/shader_reflection.h"
#include "orion-renderapi/swapchain.h"

#include "orion-math/matrix/matrix4.h"
#include "orion-math/vector/vector2.h"

#include "orion-utils/static_vector.h"

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

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        void draw(const RenderObj& obj);
        void render(const Camera& camera);

        void present_to(const RenderTarget& render_target);
        void present(RenderWindow& render_window);

        RenderWindow create_render_window(const Window& window);

        template<typename ImGuiLayerT>
        std::unique_ptr<ImGuiLayerT> create_imgui_layer()
        {
            imgui_init();
            auto layer = std::make_unique<ImGuiLayerT>();
            imgui_ = layer.get();
            return layer;
        }

        ShaderEffect create_shader_effect(const FilePath& vs_path, const FilePath& ps_path);
        ShaderPass create_shader_pass(const ShaderEffect* effect, const ShaderPassDesc& desc = {});

        std::pair<mesh_id_t, Mesh*> create_mesh(std::span<const Vertex> vertices, std::span<const vertex_index_t> indices);
        std::pair<material_id_t, Material*> create_material(const MaterialData& data);
        std::pair<texture_id_t, Texture*> create_texture(TextureInfo info, std::span<const std::byte> bytes);

        [[nodiscard]] Mesh* find_mesh(mesh_id_t mesh_id);
        [[nodiscard]] Texture* find_texture(texture_id_t texture_id);
        [[nodiscard]] Material* find_material(material_id_t material_id);

    private:
        struct FrameData {
            std::unique_ptr<CommandAllocator> command_allocator;

            std::unique_ptr<CommandList> render_command;
            UniqueFence render_fence;
            UniqueSemaphore render_semaphore;
            RenderTarget render_target;
            UniqueDescriptor render_output_descriptor;

            std::unique_ptr<CommandList> present_command;
            UniqueFence present_fence;
            UniqueSemaphore present_semaphore;

            UniqueGPUBuffer staging_buffer;
        };

        FrameData create_frame_data(frame_index_t frame_index);

        void imgui_init();
        void create_default_textures();

        static constexpr std::size_t max_render_objects = 1000;
        static constexpr std::size_t staging_buffer_size = 1024 * 1024;

        Module render_backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
        Vector2_u render_size_;

        ShaderEffect object_effect_;
        ShaderPass object_pass_;

        ShaderEffect present_effect_;
        ShaderPass present_pass_;

        DescriptorPoolHandle descriptor_pool_;

        PerFrame<FrameData> frame_data_;
        frame_index_t current_frame_index_ = 0;
        frame_index_t previous_frame_index_ = -1;

        FrameData& current_frame() { return frame_data_[current_frame_index_]; }
        FrameData& previous_frame() { return frame_data_[previous_frame_index_]; }
        void advance_frame();

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
