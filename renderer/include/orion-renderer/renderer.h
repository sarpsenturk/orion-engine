#pragma once

#include "orion-renderer/effect.h"
#include "orion-renderer/mesh.h"

#include "orion-core/dyn_lib.h"
#include "orion-core/platform.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_command.h"
#include "orion-renderapi/render_device.h"
#include "orion-renderapi/shader_reflection.h"

#include "orion-math/vector/vector2.h"

namespace orion
{
    struct RendererDesc {
        const char* backend = nullptr;
        Vector2_u render_size;
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto& mesh_builder() { return mesh_builder_; }
        [[nodiscard]] auto& effect_compiler() { return effect_compiler_; }

    private:
        Module render_backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
        std::unique_ptr<CommandAllocator> command_allocator_;

        MeshBuilder mesh_builder_;
        std::unique_ptr<ShaderReflector> shader_reflector_;
        EffectCompiler effect_compiler_;
    };
} // namespace orion
