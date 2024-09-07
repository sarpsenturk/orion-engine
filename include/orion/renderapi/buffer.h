#pragma once

#include <cstddef>

namespace orion
{
    enum class BufferUsage {
        Vertex,
        Index,
    };

    struct BufferDesc {
        std::size_t size;
        BufferUsage usage;
        bool cpu_visible;
    };
} // namespace orion
