#pragma once

#include <cstddef>

namespace orion
{
    enum class BufferUsage {
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
    };

    struct BufferDesc {
        std::size_t size;
        BufferUsage usage;
        bool cpu_visible;
    };
} // namespace orion
