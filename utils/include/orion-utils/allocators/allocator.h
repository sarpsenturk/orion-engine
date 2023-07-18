#pragma once

#include <cstddef>

namespace orion
{
    struct Allocation {
        void* ptr;
        std::size_t size;
    };
} // namespace orion
