#pragma once

#include "orion-renderapi/handles.h"
#include "orion-renderapi/types.h"

#include "orion-math/vector/vector3.h"
#include "orion-math/vector/vector4.h"

#include <array>

namespace orion
{
    struct Vertex {
        Vector3_f position;
        Vector4_f color;

        static constexpr auto vertex_bindings()
        {
            std::array attributes{
                VertexAttributeDesc{.name = "POSITION", .format = Format::R32G32B32_Float},
                VertexAttributeDesc{.name = "COLOR", .format = Format::R32G32B32A32_Float},
            };
            return std::array{
                VertexBinding(attributes, InputRate::Vertex),
            };
        }
    };

    class Mesh
    {
    public:
        Mesh(GPUBufferHandle vertex_buffer, GPUBufferHandle index_buffer);

        [[nodiscard]] auto vertex_buffer() const noexcept { return vertex_buffer_; }
        [[nodiscard]] auto index_buffer() const noexcept { return index_buffer_; }

    private:
        GPUBufferHandle vertex_buffer_;
        GPUBufferHandle index_buffer_;
    };
} // namespace orion
