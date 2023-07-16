#pragma once

#include <functional>

namespace orion
{
    template<typename T>
    std::size_t hash_combine(std::size_t seed, const T& value)
    {
        auto hasher = std::hash<T>{};
        return seed ^ hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
} // namespace orion
