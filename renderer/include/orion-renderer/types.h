#pragma once

#include "orion-math/matrix/matrix4.h"

namespace orion
{
    struct SceneCBuffer {
        Matrix4_f view_projection;
    };

    struct RenderObjBuffer {
        Matrix4_f transform;
    };
} // namespace orion
