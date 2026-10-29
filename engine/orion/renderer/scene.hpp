#pragma once

#include "material.hpp"
#include "mesh.hpp"

#include <glm/mat4x4.hpp>

namespace orion
{
    struct RenderInstance {
        Mesh mesh;
        Material material;
        glm::mat4 model;
    };

    struct Scene {
        std::span<const RenderInstance> instances;
    };
} // namespace orion
