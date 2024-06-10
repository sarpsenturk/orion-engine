#include "orion-renderer/buffer.h"

#include "orion-renderer/frame.h"

#include "orion-renderapi/render_device.h"

#include "orion-utils/assertion.h"

#include <cstring>

namespace orion
{
    DynamicGPUBuffer::DynamicGPUBuffer(UniqueGPUBuffer buffer, std::size_t size)
        : buffer_(std::move(buffer))
        , size_(size)
    {
    }

    BufferDescriptorDesc DynamicGPUBuffer::descriptor_desc(std::uint32_t index) const
    {
        return {
            .buffer_handle = buffer_.get(),
            .region = {
                .size = size_,
                .offset = size_ * index,
            },
        };
    }

    void DynamicGPUBuffer::update(RenderDevice* device, std::span<const std::byte> bytes, std::size_t offset)
    {
        ORION_ASSERT(bytes.size_bytes() <= size_);
        index_ = (index_ + 1) % frames_in_flight;
        // TODO: Should be persistently mapped if we're going to use host visible buffers
        //  but I'm not sure of that yet :/
        void* dst = device->map(buffer_.get());
        const auto frame_offset = index_ * size_;
        dst = static_cast<char*>(dst) + frame_offset + offset;
        std::memcpy(dst, bytes.data(), bytes.size_bytes());
        device->unmap(buffer_.get());
    }

    DynamicGPUBuffer create_dynamic_buffer(RenderDevice* device, std::size_t size, GPUBufferUsageFlags usage)
    {
        const auto total_size = size * frames_in_flight;
        auto buffer = device->create_buffer({
            .size = total_size,
            .usage = usage,
            .host_visible = true, // TODO: Reconsider this
        });
        return {device->to_unique(buffer), size};
    }
} // namespace orion
