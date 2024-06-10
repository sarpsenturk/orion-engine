#pragma once

#include "orion-utils/bitflag.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace orion
{
    ORION_BITFLAG(ShaderStageFlags, std::uint8_t){
        Vertex = 0x1,
        Pixel = 0x2,
        All = Vertex | Pixel,
    };

    struct ShaderModuleDesc {
        std::span<const std::byte> byte_code;
    };

    enum class ShaderObjectType {
        SpirV,
        DXIL
    };
} // namespace orion
