#pragma once

#include "types.h"

#include <span>

namespace orion
{
    struct DescriptorPoolSize {
        DescriptorType type;
        std::uint32_t count;
    };

    struct DescriptorPoolDesc {
        std::uint32_t max_sets;
        std::span<const DescriptorPoolSize> pool_sizes;
    };
} // namespace orion
