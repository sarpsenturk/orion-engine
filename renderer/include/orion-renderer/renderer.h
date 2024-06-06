#pragma once

#include "orion-renderer/camera.h"
#include "orion-renderer/config.h"
#include "orion-renderer/effect.h"
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

        [[nodiscard]] auto& effect_compiler() { return effect_compiler_; }

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
        std::pair<mesh_id_t, Mesh*> create_mesh(std::span<const Vertex> vertices, std::span<const vertex_index_t> indices);
        std::pair<material_id_t, Material*> create_material(const Effect* effect, const MaterialData& data);
        std::pair<texture_id_t, Texture*> create_texture(TextureInfo info, std::span<const std::byte> bytes);

        [[nodiscard]] Mesh* find_mesh(mesh_id_t mesh_id);
        [[nodiscard]] Texture* find_texture(texture_id_t texture_id);
        [[nodiscard]] Material* find_material(material_id_t material_id);

    private:
        void imgui_init();
        void create_default_textures();

        Module render_backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        DescriptorLayoutHandle frame_descriptor_layout_;
        DescriptorLayoutHandle material_descriptor_layout_;
        DescriptorLayoutHandle object_descriptor_layout_;
        PipelineLayoutHandle pipeline_layout_;
        EffectCompiler effect_compiler_;

        Vector2_u render_size_;

        DescriptorLayoutHandle present_descriptor_layout_;
        PipelineLayoutHandle present_pipeline_layout_;
        Effect present_effect_;
        SamplerHandle present_sampler_;

        RenderContext render_context_;

        DescriptorPoolHandle material_descriptor_pool_;

        std::vector<std::pair<mesh_id_t, Mesh>> meshes_;
        std::vector<std::pair<material_id_t, Material>> materials_;
        std::vector<std::pair<texture_id_t, Texture>> textures_;

        std::vector<RenderObj> objects_;

        ImGuiLayer* imgui_;
    };
} // namespace orion
