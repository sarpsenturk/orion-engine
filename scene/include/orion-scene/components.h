#pragma once

/*
 * This file contains all builtin components used in orion
 */

#include "orion-math/matrix/matrix4.h"
#include "orion-math/vector/vector3.h"

#include <string>

namespace orion
{
    struct TransformComponent {
        Matrix4_f transform = Matrix4_f::identity();

        [[nodiscard]] Vector3_f position() const noexcept { return {transform(3, 0), transform(3, 1), transform(3, 2)}; }

        void translate(const Vector3_f& translation);
    };

    struct TagComponent {
        std::string tag;
    };

    class Mesh;
    struct MeshComponent {
        const Mesh* mesh;
    };
} // namespace orion
