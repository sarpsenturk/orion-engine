#pragma once

#include "orion/rhi/format.hpp"
#include "orion/rhi/handle.hpp"

namespace orion
{
    enum class RHIImageLayout {
        Undefined = 0,
        RenderTarget,
        PresentSrc,
    };

    struct RHIRenderTargetViewDesc {
        RHIImage image;
        RHIFormat format;
    };
} // namespace orion
