#pragma once

#include "orion-renderapi/handles.h"

#include <functional>
#include <memory>

namespace orion
{
    // Forward declare
    class RenderDevice;

    void device_destroy(RenderDevice* device, RenderPassHandle render_pass_handle);
    void device_destroy(RenderDevice* device, FramebufferHandle framebuffer_handle);
    void device_destroy(RenderDevice* device, ShaderModuleHandle shader_module_handle);
    void device_destroy(RenderDevice* device, DescriptorLayoutHandle descriptor_layout_handle);
    void device_destroy(RenderDevice* device, DescriptorHandle descriptor_handle);
    void device_destroy(RenderDevice* device, PipelineLayoutHandle pipeline_layout_handle);
    void device_destroy(RenderDevice* device, PipelineHandle pipeline_handle);
    void device_destroy(RenderDevice* device, GPUBufferHandle buffer_handle);
    void device_destroy(RenderDevice* device, ImageHandle image_handle);
    void device_destroy(RenderDevice* device, ImageViewHandle image_view_handle);
    void device_destroy(RenderDevice* device, SamplerHandle sampler_handle);
    void device_destroy(RenderDevice* device, FenceHandle fence_handle);
    void device_destroy(RenderDevice* device, SemaphoreHandle semaphore_handle);

    template<typename Tag>
    class UniqueDeviceHandle
    {
    public:
        using handle_type = RenderDeviceHandle<Tag>;

        UniqueDeviceHandle() = default;
        UniqueDeviceHandle(handle_type handle, RenderDevice* device)
            : device_(device)
            , handle_(handle)
        {
        }
        UniqueDeviceHandle(const UniqueDeviceHandle&) = delete;
        UniqueDeviceHandle(UniqueDeviceHandle&& other) noexcept
            : device_(other.device_)
            , handle_(std::exchange(other.handle_, handle_type::invalid()))
        {
        }
        UniqueDeviceHandle& operator=(const UniqueDeviceHandle&) = delete;
        UniqueDeviceHandle& operator=(UniqueDeviceHandle&& other) noexcept
        {
            reset(other.release());
            device_ = other.device_;
            return *this;
        }
        ~UniqueDeviceHandle()
        {
            if (handle_.is_valid()) {
                device_destroy(device_, handle_);
            }
        }

        void reset(handle_type new_handle)
        {
            handle_type old = std::exchange(handle_, new_handle);
            if (old.is_valid()) {
                device_destroy(device_, handle_);
            }
        }

        handle_type release() noexcept
        {
            return std::exchange(handle_, handle_type::invalid());
        }

        handle_type get() const noexcept
        {
            return handle_;
        }

    private:
        RenderDevice* device_ = nullptr;
        handle_type handle_ = handle_type::invalid();
    };

    using UniqueRenderPass = UniqueDeviceHandle<RenderPassHandle_tag>;
    using UniqueFramebuffer = UniqueDeviceHandle<FramebufferHandle_tag>;
    using UniqueShaderModule = UniqueDeviceHandle<ShaderModuleHandle_tag>;
    using UniquePipelineLayout = UniqueDeviceHandle<PipelineLayoutHandle_tag>;
    using UniqueDescriptorLayout = UniqueDeviceHandle<DescriptorLayoutHandle_tag>;
    using UniqueDescriptor = UniqueDeviceHandle<DescriptorHandle_tag>;
    using UniquePipeline = UniqueDeviceHandle<PipelineHandle_tag>;
    using UniqueGPUBuffer = UniqueDeviceHandle<GPUBufferHandle_tag>;
    using UniqueImage = UniqueDeviceHandle<ImageHandle_tag>;
    using UniqueImageView = UniqueDeviceHandle<ImageViewHandle_tag>;
    using UniqueSampler = UniqueDeviceHandle<SamplerHandle_tag>;
    using UniqueFence = UniqueDeviceHandle<FenceHandle_tag>;
    using UniqueSemaphore = UniqueDeviceHandle<SemaphoreHandle_tag>;
} // namespace orion
