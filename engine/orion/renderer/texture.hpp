#pragma once

#include <cstdint>

namespace orion
{
    struct Texture {
        std::uint64_t handle = 0ll;
    };

    struct TextureDesc {
        std::uint32_t width  = 0u;
        std::uint32_t height = 0u;
        std::uint32_t depth  = 1u;
    };
} // namespace orion
