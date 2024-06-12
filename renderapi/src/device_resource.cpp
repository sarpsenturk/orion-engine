#include "orion-renderapi/device_resource.h"

#include "orion-renderapi/render_device.h"

namespace orion
{
    template<typename HandleType>
    void device_destroy(RenderDevice* device, HandleType handle)
    {
        device->destroy(handle);
    }

    template void device_destroy(RenderDevice*, SwapchainHandle);
    template void device_destroy(RenderDevice*, RenderPassHandle);
    template void device_destroy(RenderDevice*, ShaderModuleHandle);
    template void device_destroy(RenderDevice*, DescriptorLayoutHandle);
    template void device_destroy(RenderDevice*, DescriptorPoolHandle);
    template void device_destroy(RenderDevice*, DescriptorHandle);
    template void device_destroy(RenderDevice*, PipelineLayoutHandle);
    template void device_destroy(RenderDevice*, PipelineHandle);
    template void device_destroy(RenderDevice*, GPUBufferHandle);
    template void device_destroy(RenderDevice*, ImageHandle);
    template void device_destroy(RenderDevice*, ImageViewHandle);
    template void device_destroy(RenderDevice*, FramebufferHandle);
    template void device_destroy(RenderDevice*, SamplerHandle);
    template void device_destroy(RenderDevice*, FenceHandle);
    template void device_destroy(RenderDevice*, SemaphoreHandle);
} // namespace orion
