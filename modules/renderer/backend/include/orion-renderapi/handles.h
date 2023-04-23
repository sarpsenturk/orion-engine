#pragma once

#include "orion-core/handle.h"

#include <variant> // std::variant

/*
 * All handles to be used in the render backend should be defined here.
 * This is to ensure we don't get odr violations.
 */

namespace orion
{
    ORION_DEFINE_HANDLE(SwapchainHandle);
    ORION_DEFINE_HANDLE(ShaderModuleHandle);
    ORION_DEFINE_HANDLE(PipelineHandle);
    ORION_DEFINE_HANDLE(GPUBufferHandle);
    ORION_DEFINE_HANDLE(CommandBufferHandle);

    using RenderTargetHandle = std::variant<SwapchainHandle>;
} // namespace orion
