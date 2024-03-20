#include "orion-renderapi/device_resource.h"

#include "orion-renderapi/render_device.h"

namespace orion
{
    void device_destroy(RenderDevice* device, RenderPassHandle render_pass_handle)
    {
        device->destroy(render_pass_handle);
    }
    void orion::device_destroy(RenderDevice* device, FramebufferHandle framebuffer_handle)
    {
        device->destroy(framebuffer_handle);
    }
    void orion::device_destroy(RenderDevice* device, ShaderModuleHandle shader_module_handle)
    {
        device->destroy(shader_module_handle);
    }
    void orion::device_destroy(RenderDevice* device, DescriptorLayoutHandle descriptor_layout_handle)
    {
        device->destroy(descriptor_layout_handle);
    }
    void orion::device_destroy(RenderDevice* device, DescriptorHandle descriptor_handle)
    {
        device->destroy(descriptor_handle);
    }
    void orion::device_destroy(RenderDevice* device, PipelineLayoutHandle pipeline_layout_handle)
    {
        device->destroy(pipeline_layout_handle);
    }
    void orion::device_destroy(RenderDevice* device, PipelineHandle pipeline_handle)
    {
        device->destroy(pipeline_handle);
    }
    void orion::device_destroy(RenderDevice* device, GPUBufferHandle buffer_handle)
    {
        device->destroy(buffer_handle);
    }
    void orion::device_destroy(RenderDevice* device, ImageHandle image_handle)
    {
        device->destroy(image_handle);
    }
    void orion::device_destroy(RenderDevice* device, ImageViewHandle image_view_handle)
    {
        device->destroy(image_view_handle);
    }
    void orion::device_destroy(RenderDevice* device, SamplerHandle sampler_handle)
    {
        device->destroy(sampler_handle);
    }
    void orion::device_destroy(RenderDevice* device, FenceHandle fence_handle)
    {
        device->destroy(fence_handle);
    }
    void orion::device_destroy(RenderDevice* device, SemaphoreHandle semaphore_handle)
    {
        device->destroy(semaphore_handle);
    }
} // namespace orion
