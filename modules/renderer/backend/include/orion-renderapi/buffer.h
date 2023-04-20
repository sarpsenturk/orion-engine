#pragma once

#include "handles.h"
#include "types.h"

namespace orion
{
    struct GPUBufferDesc {
        std::size_t size = 0;
        GPUBufferUsageFlags usage = {};
    };

    class GPUBuffer
    {
    public:
        GPUBuffer() = default;
        GPUBuffer(GPUBufferHandle handle, GPUBufferDesc desc);

        [[nodiscard]] auto handle() const noexcept { return handle_; }
        [[nodiscard]] auto size() const noexcept { return desc_.size; }
        [[nodiscard]] auto usage() const noexcept { return desc_.usage; }
        [[nodiscard]] auto& desc() const noexcept { return desc_; }

    private:
        GPUBufferHandle handle_;
        GPUBufferDesc desc_;
    };
} // namespace orion
