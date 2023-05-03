#pragma once

#include "handles.h"
#include "types.h"

namespace orion
{
    struct GPUBufferDesc {
        std::size_t size = 0;
        GPUBufferUsageFlags usage = {};
        bool host_visible = false;
    };
} // namespace orion
