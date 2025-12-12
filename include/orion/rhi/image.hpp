#pragma once

#include "orion/rhi/format.hpp"
#include "orion/rhi/handle.hpp"

namespace orion
{
    struct RHIRenderTargetViewDesc {
        RHIImage image;
        RHIFormat format;
    };
} // namespace orion
