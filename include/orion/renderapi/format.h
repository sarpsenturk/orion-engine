#pragma once

#include <cstddef>

namespace orion
{
    enum class Format {
        Undefined = 0,
        B8G8R8A8_Unorm,
        R32G32B32_Float,
    };

    std::size_t format_byte_size(Format format);
} // namespace orion
