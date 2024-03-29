#pragma once

#include "concepts.h"

namespace orion
{
    template<LessThanComparable T>
    const T& min(const T& a, const T& b)
    {
        return a < b ? a : b;
    }

    template<LessThanComparable T>
    const T& max(const T& a, const T& b)
    {
        return a < b ? b : a;
    }
} // namespace orion
