#include "orion-renderapi/buffer.h"

#include "orion-renderapi/render_device.h"

#include "orion-utils/assertion.h"

#include <cstring>
#include <utility>

namespace orion
{
    GPUBuffer::GPUBuffer(RenderDevice* device, const GPUBufferDesc& desc)
        : device_(device)
        , desc_(desc)
        , handle_(desc.size > 0 ? device->create_buffer(desc) : GPUBufferHandle::invalid())
    {
    }

    GPUBuffer::GPUBuffer(GPUBuffer&& other) noexcept
        : device_(other.device_)
        , desc_(other.desc_)
        , handle_(std::exchange(other.handle_, GPUBufferHandle::invalid()))
    {
    }

    GPUBuffer& GPUBuffer::operator=(GPUBuffer&& other) noexcept
    {
        if (&other != this) {
            device_ = other.device_;
            desc_ = other.desc_;
            handle_ = std::exchange(other.handle_, GPUBufferHandle::invalid());
        }
        return *this;
    }

    GPUBuffer::~GPUBuffer()
    {
        if (handle_.is_valid()) {
            device_->destroy(handle_);
        }
    }

    void GPUBuffer::recreate(const GPUBufferDesc& desc)
    {
        if (handle_.is_valid()) {
            device_->destroy(handle_);
        }
        handle_ = device_->create_buffer(desc);
        desc_ = desc;
    }

    BufferBindingDesc GPUBuffer::descriptor_value(BufferRegion region) const
    {
        ORION_ASSERT(region.offset < size() && "buffer region offset can't be larger than buffer size");

        if (region.size == buffer_whole_size) {
            region.size = size() - region.offset;
        }

        return {
            .buffer_handle = handle_,
            .region = region,
        };
    }

    DescriptorBinding GPUBuffer::descriptor_binding(std::uint32_t binding, const BufferRegion& region) const
    {
        return {
            .binding = binding,
            .binding_type = get_binding_type(usage()),
            .binding_value = descriptor_value(region),
        };
    }

    MappedGPUBuffer::MappedGPUBuffer(RenderDevice* device, std::size_t size, GPUBufferUsageFlags usage)
        : GPUBuffer(device, {.size = size, .usage = usage, .host_visible = true})
        , ptr_(is_empty() ? nullptr : map())
    {
    }

    MappedGPUBuffer::MappedGPUBuffer(MappedGPUBuffer&& other) noexcept
        : GPUBuffer(std::move(other))
        , ptr_(std::exchange(other.ptr_, nullptr))
    {
    }

    MappedGPUBuffer& MappedGPUBuffer::operator=(MappedGPUBuffer&& other) noexcept
    {
        if (&other != this) {
            GPUBuffer::operator=(std::move(other));
            ptr_ = std::exchange(other.ptr_, nullptr);
        }
        return *this;
    }

    MappedGPUBuffer::~MappedGPUBuffer()
    {
        if (ptr_ != nullptr) {
            unmap();
        }
    }

    void* MappedGPUBuffer::map()
    {
        ORION_ASSERT(!is_empty() && "trying to map() an empty buffer");
        if (ptr_ != nullptr) {
            return ptr_;
        }
        return device()->map(handle());
    }

    void MappedGPUBuffer::unmap()
    {
        ORION_ASSERT(ptr_ != nullptr && "trying to unmap() buffer which was not previously mapped with map()");
        device()->unmap(handle());
    }

    void MappedGPUBuffer::resize(std::size_t new_size)
    {
        if (ptr_ != nullptr) {
            unmap();
            ptr_ = nullptr;
        }
        auto new_desc = desc();
        new_desc.size = new_size;
        recreate(new_desc);

        if (!is_empty()) {
            ptr_ = map();
        }
    }

    void MappedGPUBuffer::upload(std::span<const std::byte> bytes, std::size_t offset)
    {
        ORION_ASSERT(bytes.size() + offset <= size());
        std::memcpy(static_cast<char*>(ptr_) + offset, bytes.data(), bytes.size());
    }

    void MappedGPUBuffer::upload_grow(std::span<const std::byte> bytes, std::size_t offset)
    {
        const auto data_size = bytes.size() + offset;
        if (size() < data_size) {
            resize(data_size);
        }
        std::memcpy(static_cast<char*>(ptr_) + offset, bytes.data(), bytes.size());
    }
} // namespace orion
