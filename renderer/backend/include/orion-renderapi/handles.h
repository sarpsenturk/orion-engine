#pragma once

#include "orion-core/handle.h"

#include <variant> // std::variant

/*
 * All handles to be used in the render backend should be defined here.
 */

namespace orion
{
    ORION_DEFINE_HANDLE(SurfaceHandle);
    ORION_DEFINE_HANDLE(SwapchainHandle);
    ORION_DEFINE_HANDLE(RenderPassHandle);
    ORION_DEFINE_HANDLE(ShaderModuleHandle);
    ORION_DEFINE_HANDLE(PipelineHandle);
    ORION_DEFINE_HANDLE(GPUBufferHandle);
    ORION_DEFINE_HANDLE(CommandPoolHandle);
    ORION_DEFINE_HANDLE(CommandBufferHandle);
    ORION_DEFINE_HANDLE(DescriptorPoolHandle);
    ORION_DEFINE_HANDLE(DescriptorSetHandle);
    ORION_DEFINE_HANDLE(SemaphoreHandle);
    ORION_DEFINE_HANDLE(FenceHandle);
    ORION_DEFINE_HANDLE(ImageHandle);
    ORION_DEFINE_HANDLE(ImageViewHandle);
    ORION_DEFINE_HANDLE(FramebufferHandle);
} // namespace orion
