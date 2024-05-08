#pragma once

#include "orion-renderapi/defs.h"
#include "orion-renderapi/handles.h"

#include <cstddef>
#include <span>

namespace orion
{
    // Forward declare
    class RenderDevice;

    class GPUBuffer
    {
    public:
        GPUBuffer(RenderDevice* device, const GPUBufferDesc& desc);
        GPUBuffer(const GPUBuffer&) = delete;
        GPUBuffer(GPUBuffer&& other) noexcept;
        GPUBuffer& operator=(const GPUBuffer&) = delete;
        GPUBuffer& operator=(GPUBuffer&& other) noexcept;
        virtual ~GPUBuffer();

        [[nodiscard]] RenderDevice* device() const noexcept { return device_; }
        [[nodiscard]] std::size_t size() const noexcept { return desc_.size; }
        [[nodiscard]] GPUBufferUsageFlags usage() const noexcept { return desc_.usage; }
        [[nodiscard]] bool host_visible() const noexcept { return desc_.host_visible; }
        [[nodiscard]] const GPUBufferDesc& desc() const noexcept { return desc_; }
        [[nodiscard]] GPUBufferHandle handle() const noexcept { return handle_; }

        [[nodiscard]] bool is_empty() const noexcept { return desc_.size == 0; }

    protected:
        void recreate(const GPUBufferDesc& desc);

    private:
        RenderDevice* device_;
        GPUBufferDesc desc_;
        GPUBufferHandle handle_;
    };

    class MappedGPUBuffer : public GPUBuffer
    {
    public:
        MappedGPUBuffer(RenderDevice* device, std::size_t size, GPUBufferUsageFlags usage);
        MappedGPUBuffer(const MappedGPUBuffer&) = delete;
        MappedGPUBuffer(MappedGPUBuffer&& other) noexcept;
        MappedGPUBuffer& operator=(const MappedGPUBuffer&) = delete;
        MappedGPUBuffer& operator=(MappedGPUBuffer&& other) noexcept;
        ~MappedGPUBuffer() override;

        [[nodiscard]] void* ptr() const noexcept { return ptr_; }

        void* map();
        void unmap();
        void resize(std::size_t new_size);

        void upload(std::span<const std::byte> bytes, std::size_t offset = 0ull);
        void upload_grow(std::span<const std::byte> bytes, std::size_t offset = 0ull);

    private:
        void* ptr_;
    };
} // namespace orion
