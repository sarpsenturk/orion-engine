#pragma once

/*
 * This file contains all builtin components used in orion
 */

#include "orion-math/matrix/matrix4.h"

#include <string>

namespace orion
{
    struct TransformComponent {
        Matrix4 transform;
    };

    struct TagComponent {
        std::string tag;
    };

    class Mesh;
    struct MeshComponent {
        Mesh* mesh;
    };
} // namespace orion
