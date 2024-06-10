#pragma once

#include "orion-utils/bitflag.h"

#include <cstddef>
#include <cstdint>

namespace orion
{
    ORION_BITFLAG(GPUBufferUsageFlags, std::uint8_t){
        VertexBuffer = 0x1,
        IndexBuffer = 0x2,
        ConstantBuffer = 0x4,
        TransferSrc = 0x8,
        TransferDst = 0x10,
        StorageBuffer = 0x20,
        Transfer = TransferSrc | TransferDst,
    };

    struct GPUBufferDesc {
        std::size_t size = 0;
        GPUBufferUsageFlags usage = {};
        bool host_visible = false;
    };

    struct BufferRegion {
        std::size_t size;
        std::size_t offset = 0ull;
    };
} // namespace orion
