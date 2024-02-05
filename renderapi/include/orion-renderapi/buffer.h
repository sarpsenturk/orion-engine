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

        [[nodiscard]] GPUBufferHandle handle() const noexcept { return buffer_; }
        [[nodiscard]] std::size_t size() const noexcept { return size_; }
        [[nodiscard]] GPUBufferUsageFlags usage() const noexcept { return usage_; }
        [[nodiscard]] void* ptr() noexcept { return ptr_; }

        [[nodiscard]] bool needs_resize(std::size_t new_size) const noexcept { return new_size != size_; }
        [[nodiscard]] bool needs_grow(std::size_t new_size) const noexcept { return new_size > size_; }
        [[nodiscard]] bool needs_shrink(std::size_t new_size) const noexcept { return new_size < size_; }

        [[nodiscard]] BufferBindingDesc binding_desc(std::size_t offset = 0ull) const noexcept;
        [[nodiscard]] DescriptorBinding descriptor_binding(std::uint32_t binding, std::size_t offset = 0ull) const noexcept;

        void resize(std::size_t new_size);
        void grow(std::size_t new_size);
        void shrink(std::size_t new_size);

        void upload(std::span<const std::byte> data);
        void upload_resize(std::span<const std::byte> data);
        void upload_grow(std::span<const std::byte> data);
        void upload_shrink(std::span<const std::byte> data);

    private:
        void recreate(size_t new_size);

        RenderDevice* device_;
        std::size_t size_ = 0;
        GPUBufferUsageFlags usage_;
        GPUBufferHandle buffer_ = GPUBufferHandle::invalid();
        void* ptr_ = nullptr;
    };
} // namespace orion
