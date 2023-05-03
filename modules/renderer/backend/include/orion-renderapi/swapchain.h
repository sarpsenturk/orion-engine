#pragma once

#include "handles.h"
#include "types.h"

namespace orion
{
    struct SwapchainDesc {
        std::uint32_t image_count;
        Format image_format;
        math::Vector2_u image_size;
    };
} // namespace orion
