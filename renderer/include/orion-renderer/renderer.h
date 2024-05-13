#pragma once

#include "orion-renderer/config.h"
#include "orion-renderer/effect.h"
#include "orion-renderer/material.h"
#include "orion-renderer/mesh.h"
#include "orion-renderer/render_target.h"

#include "orion-core/dyn_lib.h"
#include "orion-core/platform.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_command.h"
#include "orion-renderapi/render_device.h"
#include "orion-renderapi/shader_reflection.h"

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

    struct FrameInFlight {
        std::unique_ptr<CommandAllocator> command_allocator;
        std::unique_ptr<CommandList> command_list;

        FenceHandle render_fence;
        SemaphoreHandle render_semaphore;

        RenderTarget render_target;
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto& mesh_builder() { return mesh_builder_; }
        [[nodiscard]] auto& effect_compiler() { return effect_compiler_; }

        void draw(RenderObj obj);
        void render();

        [[nodiscard]] frame_index_t current_frame_index() const noexcept { return current_frame_index_; }
        [[nodiscard]] frame_index_t previous_frame_index() const noexcept { return previous_frame_index_; }

        [[nodiscard]] auto& current_frame() { return frames_in_flight_[current_frame_index_]; }
        [[nodiscard]] auto& previous_frame() { return frames_in_flight_[previous_frame_index_]; }

    private:
        void advance_frame();

        Module render_backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
        std::unique_ptr<CommandAllocator> command_allocator_;

        MeshBuilder mesh_builder_;
        std::unique_ptr<ShaderReflector> shader_reflector_;
        EffectCompiler effect_compiler_;

        Vector2_u render_size_;

        static_vector<FrameInFlight, frames_in_flight> frames_in_flight_;
        frame_index_t current_frame_index_ = 0;
        frame_index_t previous_frame_index_ = -1;

        std::vector<RenderObj> objects_;
    };
} // namespace orion
