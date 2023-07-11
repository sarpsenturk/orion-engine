#pragma once

#include <orion-renderapi/pipeline.h>

#include <orion-math/vector/vector3.h>
#include <orion-math/vector/vector4.h>

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
            };
            return std::array{
                VertexBinding(attributes, InputRate::Vertex),
            };
        }
    };
} // namespace orion
