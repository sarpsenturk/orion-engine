#pragma once

#include <glm/vec3.hpp>

#include <cstdint>

namespace orion
{
    struct Vertex {
        glm::vec3 position;
    };

    struct Mesh {
        std::uint32_t vertex_count = 0u;
        std::int32_t vertex_offset = 0u;
        std::uint32_t index_offset = 0u;
        std::uint32_t index_count  = 0u;
    };
} // namespace orion
