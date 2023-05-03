#pragma once

#include "handles.h"

#include <cstddef> // std::byte
#include <span>    // std::span

namespace orion
{
    struct ShaderModuleDesc {
        std::span<const std::byte> byte_code;
    };
} // namespace orion
