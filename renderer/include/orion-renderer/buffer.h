#pragma once

#include "orion-renderer/config.h"

#include "orion-renderapi/defs.h"
#include "orion-renderapi/device_resource.h"

#include <cstddef>
#include <span>

namespace orion
{
    class RenderDevice;

    class DynamicGPUBuffer
    {
    public:
        DynamicGPUBuffer(UniqueGPUBuffer buffer, std::size_t size);

        void update(RenderDevice* device, std::span<const std::byte> bytes, std::size_t offset = 0ull);

        [[nodiscard]] GPUBufferHandle buffer() const { return buffer_.get(); }
        [[nodiscard]] std::uint32_t index() const { return index_; }
        [[nodiscard]] BufferDescriptorDesc descriptor_desc(std::uint32_t index) const;

    private:
        UniqueGPUBuffer buffer_;
        std::size_t size_;
        std::uint32_t index_ = 0;
    };

    DynamicGPUBuffer create_dynamic_buffer(RenderDevice* device, std::size_t size, GPUBufferUsageFlags usage);
} // namespace orion
