#pragma once

#include "orion-renderapi/defs.h"
#include "orion-renderapi/device_resource.h"

#include "orion-math/vector/vector2.h"
#include "orion-math/vector/vector3.h"

#include <cstdint>
#include <span>

namespace orion
{
    class Mesh
    {
    public:
        Mesh(UniqueGPUBuffer vertex_buffer, UniqueGPUBuffer index_buffer, std::uint32_t index_count);

        [[nodiscard]] GPUBufferHandle vertex_buffer() const noexcept { return vertex_buffer_.get(); }
        [[nodiscard]] GPUBufferHandle index_buffer() const noexcept { return index_buffer_.get(); }
        [[nodiscard]] std::uint32_t index_count() const noexcept { return index_count_; }

    private:
        UniqueGPUBuffer vertex_buffer_;
        UniqueGPUBuffer index_buffer_;
        std::uint32_t index_count_;
    };

    struct Vertex {
        Vector3_f position;
        Vector2_f uv;
    };

    using vertex_index_t = std::uint32_t;
    inline constexpr auto vertex_index_type = IndexType::Uint32;
} // namespace orion
