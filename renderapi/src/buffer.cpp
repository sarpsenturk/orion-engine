#include "orion-renderapi/buffer.h"

#include "orion-renderapi/render_device.h"

#include "orion-utils/assertion.h"

#include <cstring>
#include <utility>

namespace orion
{
    MappedGPUBuffer::MappedGPUBuffer(RenderDevice* device, GPUBufferUsageFlags usage)
        : device_(device)
        , usage_(usage)
    {
        ORION_EXPECTS(device != nullptr);
    }

    MappedGPUBuffer::MappedGPUBuffer(RenderDevice* device, std::size_t size, GPUBufferUsageFlags usage)
        : device_(device)
        , size_(size)
        , usage_(usage)
        , buffer_(device->create_buffer({.size = size, .usage = usage, .host_visible = true}))
        , ptr_(device->map(buffer_))
    {
    }

    MappedGPUBuffer::MappedGPUBuffer(RenderDevice* device, std::span<const std::byte> data, GPUBufferUsageFlags usage)
        : device_(device)
        , size_(data.size_bytes())
        , usage_(usage)
        , buffer_(device->create_buffer({.size = data.size_bytes(), .usage = usage, .host_visible = true}))
        , ptr_(device->map(buffer_))
    {
        std::memcpy(ptr_, data.data(), data.size_bytes());
    }

    MappedGPUBuffer::MappedGPUBuffer(MappedGPUBuffer&& other) noexcept
        : device_(other.device_)
        , size_(other.size_)
        , usage_(other.usage_)
        , buffer_(std::exchange(other.buffer_, GPUBufferHandle::invalid()))
        , ptr_(other.ptr_)
    {
    }

    MappedGPUBuffer& MappedGPUBuffer::operator=(MappedGPUBuffer&& other) noexcept
    {
        if (&other != this) {
            device_ = other.device_;
            size_ = other.size_;
            usage_ = other.usage_;
            buffer_ = std::exchange(other.buffer_, GPUBufferHandle::invalid());
            ptr_ = other.ptr_;
        }
        return *this;
    }

    MappedGPUBuffer::~MappedGPUBuffer()
    {
        if (buffer_.is_valid()) {
            device_->unmap(buffer_);
            device_->destroy(buffer_);
        }
    }

    void MappedGPUBuffer::resize(std::size_t new_size)
    {
        if (new_size > size_) {
            if (buffer_.is_valid()) {
                device_->unmap(buffer_);
                device_->destroy(buffer_);
            }
            buffer_ = device_->create_buffer({.size = new_size, .usage = usage_, .host_visible = true});
            ptr_ = device_->map(buffer_);
            size_ = new_size;
        }
    }

    void MappedGPUBuffer::upload(std::span<const std::byte> data)
    {
        resize(data.size_bytes());
        std::memcpy(ptr_, data.data(), data.size_bytes());
    }
} // namespace orion
