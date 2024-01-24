#pragma once

#include "orion-renderapi/defs.h"
#include "orion-renderapi/handles.h"

#include <cstddef>
#include <span>

namespace orion
{
    // Forward declare
    class RenderDevice;

    class MappedGPUBuffer
    {
    public:
        MappedGPUBuffer(RenderDevice* device, GPUBufferUsageFlags usage);
        MappedGPUBuffer(RenderDevice* device, std::size_t size, GPUBufferUsageFlags usage);
        MappedGPUBuffer(RenderDevice* device, std::span<const std::byte> data, GPUBufferUsageFlags usage);
        MappedGPUBuffer(const MappedGPUBuffer&) = delete;
        MappedGPUBuffer(MappedGPUBuffer&& other) noexcept;
        MappedGPUBuffer& operator=(const MappedGPUBuffer&) = delete;
        MappedGPUBuffer& operator=(MappedGPUBuffer&& other) noexcept;
        ~MappedGPUBuffer();

        [[nodiscard]] auto handle() const noexcept { return buffer_; }
        [[nodiscard]] auto size() const noexcept { return size_; }
        [[nodiscard]] auto usage() const noexcept { return usage_; }
        [[nodiscard]] void* ptr() noexcept { return ptr_; }

        void resize(std::size_t new_size);
        void upload(std::span<const std::byte> data);

    private:
        RenderDevice* device_;
        std::size_t size_ = 0;
        GPUBufferUsageFlags usage_;
        GPUBufferHandle buffer_ = GPUBufferHandle::invalid();
        void* ptr_ = nullptr;
    };
} // namespace orion
