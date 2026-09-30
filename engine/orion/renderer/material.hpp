#pragma once

#include "orion/renderer/texture.hpp"

#include <cstdint>

namespace orion
{
    struct Material {
        std::uint64_t handle = 0ull;
    };

    struct MaterialDesc {
        Texture albedo;
    };
} // namespace orion
