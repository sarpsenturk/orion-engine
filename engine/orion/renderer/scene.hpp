#pragma once

#include "mesh.hpp"

#include <glm/mat4x4.hpp>

namespace orion
{
    struct RenderInstance {
        Mesh mesh;
        glm::mat4 model;
    };

    struct Scene {
        std::span<const RenderInstance> instances;
    };
} // namespace orion
