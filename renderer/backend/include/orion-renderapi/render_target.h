#pragma once

#include "handles.h"
#include "types.h"

#include <orion-math/vector/vector2.h>

namespace orion
{
    struct RenderTargetDesc {
        Format format;
        RenderPassHandle render_pass;
        Vector2_u size;
    };
} // namespace orion
