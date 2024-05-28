#pragma once

#include "orion-renderer/config.h"
#include "orion-renderer/effect.h"
#include "orion-renderer/material.h"
#include "orion-renderer/mesh.h"
#include "orion-renderer/render_context.h"
#include "orion-renderer/render_target.h"
#include "orion-renderer/render_window.h"

#include "orion-core/dyn_lib.h"
#include "orion-core/platform.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_command.h"
#include "orion-renderapi/render_device.h"
#include "orion-renderapi/shader_reflection.h"
#include "orion-renderapi/swapchain.h"

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
    };

    enum class DescriptorIndex : std::uint32_t {
        Frame,
        Material,
        Object
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto& mesh_builder() { return mesh_builder_; }
        [[nodiscard]] auto& effect_compiler() { return effect_compiler_; }
        [[nodiscard]] auto& material_builder() { return material_builder_; }

        void draw(RenderObj obj);
        void render();

        void present_to(const RenderTarget& render_target);
        void present(RenderWindow& render_window);

        RenderWindow create_render_window(const Window& window);

    private:
        void bind_descriptor(CommandList* command_list, DescriptorIndex index, DescriptorHandle descriptor);

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

        std::vector<RenderObj> objects_;
    };
} // namespace orion
