#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstdint>

namespace orion
{
    struct Vertex {
        glm::vec3 position;
        glm::vec2 texcoord;
    };

    struct Mesh {
        std::uint64_t handle = 0ull;
    };
} // namespace orion
