#pragma once

#include "orion-renderapi/handles.h"

#include <functional>

namespace orion
{
    // Forward declare
    class RenderDevice;

    template<typename Tag>
    struct DeviceResourceDeleter {
        using pointer = Handle<Tag, render_device_key_t>;
        using delete_fn = void (RenderDevice::*)(pointer);

        RenderDevice* device;
        delete_fn deleter;

        void operator()(pointer resource) const
        {
            if (resource) {
                std::invoke(deleter, device, resource);
            }
        }
    };

    template<typename Tag>
    using UniqueDeviceResource = std::unique_ptr<Handle<Tag>, DeviceResourceDeleter<Tag>>;

    using UniqueRenderPass = UniqueDeviceResource<RenderPassHandle_tag>;
    using UniqueFramebuffer = UniqueDeviceResource<FramebufferHandle_tag>;
    using UniqueShaderModule = UniqueDeviceResource<ShaderModuleHandle_tag>;
    using UniquePipelineLayout = UniqueDeviceResource<PipelineLayoutHandle_tag>;
    using UniqueDescriptorLayout = UniqueDeviceResource<DescriptorLayoutHandle_tag>;
    using UniqueDescriptor = UniqueDeviceResource<DescriptorHandle_tag>;
    using UniquePipeline = UniqueDeviceResource<PipelineHandle_tag>;
    using UniqueGPUBuffer = UniqueDeviceResource<GPUBufferHandle_tag>;
    using UniqueImage = UniqueDeviceResource<ImageHandle_tag>;
    using UniqueImageView = UniqueDeviceResource<ImageViewHandle_tag>;
    using UniqueSampler = UniqueDeviceResource<SamplerHandle_tag>;
    using UniqueFence = UniqueDeviceResource<FenceHandle_tag>;
    using UniqueSemaphore = UniqueDeviceResource<SemaphoreHandle_tag>;
} // namespace orion
