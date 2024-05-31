#pragma once

#include "orion-renderer/camera.h"
#include "orion-renderer/config.h"
#include "orion-renderer/effect.h"
#include "orion-renderer/material.h"
#include "orion-renderer/mesh.h"
#include "orion-renderer/render_context.h"
#include "orion-renderer/render_target.h"
#include "orion-renderer/render_window.h"

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

namespace orion
{
    struct RendererDesc {
        const char* backend = nullptr;
        Vector2_u render_size;
    };

    struct RenderObj {
        const Mesh* mesh;
        const Material* material;
        const Matrix4_f* transform;
    };

    inline constexpr std::uint32_t descriptor_index_frame = 0;
    inline constexpr std::uint32_t descriptor_index_material = 1;
    inline constexpr std::uint32_t descriptor_index_object = 2;

    struct RenderCommand {
        std::uint64_t key;

        std::uint16_t material() const noexcept;
        std::uint16_t mesh() const noexcept;

        constexpr auto operator<=>(const RenderCommand&) const noexcept = default;
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto& mesh_builder() { return mesh_builder_; }
        [[nodiscard]] auto& effect_compiler() { return effect_compiler_; }
        [[nodiscard]] auto& material_builder() { return material_builder_; }

        void draw(const RenderObj& obj);
        void render(const Camera& camera);

        void present_to(const RenderTarget& render_target);
        void present(RenderWindow& render_window);

        RenderWindow create_render_window(const Window& window);

    private:
        Module render_backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        std::unique_ptr<ShaderReflector> shader_reflector_;

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
        MeshBuilder mesh_builder_;
        MaterialBuilder material_builder_;

        std::uint16_t add_material(const Material* material);
        std::uint16_t add_mesh(const Mesh* mesh);
        std::size_t add_transform(const Matrix4_f* transform);

        void clear_scene();

        void bind_view(CommandList* command_list, const Matrix4_f& view_projection);
        void bind_material(CommandList* command_list, std::uint16_t index);
        void bind_mesh(CommandList* command_list, std::uint16_t index);
        void bind_transform(CommandList* command_list, std::size_t index);

        std::vector<RenderCommand> commands_;
        std::vector<const Material*> materials_;
        std::vector<const Mesh*> meshes_;
        std::vector<const Matrix4_f*> transforms_;
    };
} // namespace orion
