#pragma once

#include "handles.h"
#include "types.h"

#include <orion-math/vector/vector2.h>

namespace orion
{
    struct RenderTargetDesc {
        Format format;
        RenderPassHandle render_pass;
        math::Vector2_u size;
    };
} // namespace orion
