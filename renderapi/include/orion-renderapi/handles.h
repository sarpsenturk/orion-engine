#pragma once

#include "orion-core/handle.h"

#include <cstdint>

/*
 * All handles to be used in the render backend should be defined here.
 */

namespace orion
{
    using render_device_key_t = std::uint64_t;
    ORION_DEFINE_HANDLE(RenderPassHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(ShaderModuleHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(DescriptorLayoutHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(DescriptorHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(PipelineLayoutHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(PipelineHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(GPUBufferHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(ImageHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(ImageViewHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(FramebufferHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(SamplerHandle, render_device_key_t);
    ORION_DEFINE_HANDLE(FenceHandle, render_device_key_t);
} // namespace orion
