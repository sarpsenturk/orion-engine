#pragma once

#include "handles.h"
#include "types.h"

namespace orion
{
    struct ImageBarrierDesc {
        ResourceAccessFlags src_access;
        ResourceAccessFlags dst_access;
        ImageLayout old_layout;
        ImageLayout new_layout;
        ImageHandle image;
    };
} // namespace orion
