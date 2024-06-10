#pragma once

#include "orion-math/vector/vector2.h"

namespace orion
{
    enum class IndexType {
        Uint16,
        Uint32,
    };

    enum class PipelineBindPoint {
        Graphics,
        Compute
    };

    struct Rect2D {
        Vector2_i offset;
        Vector2_u size;
    };
} // namespace orion
