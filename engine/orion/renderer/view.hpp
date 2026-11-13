#pragma once

#include "draw_packet.hpp"

#include "orion/gpu/gpu.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <cstddef>
#include <cstdint>

struct ImDrawData;

namespace orion
{
    // Valid only within the Frame that created it, until that frame is reset.
    struct ViewIndex {
        std::uint32_t value;
    };

    struct ViewDesc {
        glm::mat4 view       = glm::mat4{1.0f};
        glm::mat4 projection = glm::mat4{1.0f};
        gpu::Viewport viewport;
        gpu::Rect2D scissor;
    };

    // Transforms and camera position use the same world coordinates, with a zero render origin.
    struct alignas(16) GpuView {
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 view_projection;
        glm::vec4 camera_world_position;
    };
    static_assert(sizeof(GpuView) == 208z);
    static_assert(offsetof(GpuView, view) == 0z);
    static_assert(offsetof(GpuView, projection) == 64z);
    static_assert(offsetof(GpuView, view_projection) == 128z);
    static_assert(offsetof(GpuView, camera_world_position) == 192z);

    struct RenderView {
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 view_projection;
        glm::vec4 camera_world_position;
        gpu::Viewport viewport;
        gpu::Rect2D scissor;
        gpu::Address gpu_address;
        // Opaque pass draws
        OpaqueDrawList opaque_draws;
        // Imgui pass draws
        ImDrawData* imgui_draw_data = nullptr;
    };
} // namespace orion
