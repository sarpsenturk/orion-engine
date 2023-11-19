#pragma once

#include "orion-renderapi/defs.h"

#include "orion-math/vector/vector3.h"
#include "orion-math/vector/vector4.h"

#include <array>
#include <span>

namespace orion
{
    struct Vertex {
        Vector3_f position;
        Vector4_f color;

        static auto vertex_attributes()
        {
            static const auto attributes = std::array{
                VertexAttributeDesc{.name = "POSITION", .format = Format::R32G32B32_Float},
                VertexAttributeDesc{.name = "COLOR", .format = Format::R32G32B32A32_Float},
            };
            return std::span{attributes};
        }

        static auto vertex_bindings()
        {
            static const auto bindings = std::array{VertexBinding(vertex_attributes(), InputRate::Vertex)};
            return std::span{bindings};
        }
    };

} // namespace orion
