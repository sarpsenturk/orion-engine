#pragma once

#include "orion-renderapi/handles.h"

#include <functional>
#include <memory>

namespace orion
{
    // Forward declare
    class RenderDevice;

    template<typename HandleType>
    void device_destroy(RenderDevice* device, HandleType handle);

    extern template void device_destroy(RenderDevice*, ShaderModuleHandle);
    extern template void device_destroy(RenderDevice*, DescriptorLayoutHandle);
    extern template void device_destroy(RenderDevice*, DescriptorPoolHandle);
    extern template void device_destroy(RenderDevice*, DescriptorHandle);
    extern template void device_destroy(RenderDevice*, PipelineLayoutHandle);
    extern template void device_destroy(RenderDevice*, PipelineHandle);
    extern template void device_destroy(RenderDevice*, GPUBufferHandle);
    extern template void device_destroy(RenderDevice*, ImageHandle);
    extern template void device_destroy(RenderDevice*, ImageViewHandle);
    extern template void device_destroy(RenderDevice*, SamplerHandle);
    extern template void device_destroy(RenderDevice*, FenceHandle);
    extern template void device_destroy(RenderDevice*, SemaphoreHandle);

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
                device_destroy(device_, old);
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

    using UniqueShaderModule = UniqueDeviceHandle<ShaderModuleHandle_tag>;
    using UniquePipelineLayout = UniqueDeviceHandle<PipelineLayoutHandle_tag>;
    using UniqueDescriptorLayout = UniqueDeviceHandle<DescriptorLayoutHandle_tag>;
    using UniqueDescriptorPool = UniqueDeviceHandle<DescriptorPoolHandle_tag>;
    using UniqueDescriptor = UniqueDeviceHandle<DescriptorHandle_tag>;
    using UniquePipeline = UniqueDeviceHandle<PipelineHandle_tag>;
    using UniqueGPUBuffer = UniqueDeviceHandle<GPUBufferHandle_tag>;
    using UniqueImage = UniqueDeviceHandle<ImageHandle_tag>;
    using UniqueImageView = UniqueDeviceHandle<ImageViewHandle_tag>;
    using UniqueSampler = UniqueDeviceHandle<SamplerHandle_tag>;
    using UniqueFence = UniqueDeviceHandle<FenceHandle_tag>;
    using UniqueSemaphore = UniqueDeviceHandle<SemaphoreHandle_tag>;
} // namespace orion
