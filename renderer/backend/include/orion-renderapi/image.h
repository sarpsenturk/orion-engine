#pragma once

#include "types.h"

#include <orion-math/vector/vector3.h>

namespace orion
{
    struct ImageDesc {
        ImageType type;
        Format format;
        Vector3_u size;
        ImageTiling tiling;
        ImageUsageFlags usage;
    };

    struct ImageViewDesc {
        ImageHandle image;
        ImageViewType type;
        Format format;
    };
} // namespace orion
