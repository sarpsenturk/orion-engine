#pragma once

#include "orion/math/vector.h"

namespace orion
{
    template<typename T>
    using Vector3_t = Vector<T, 3>;

    using Vector3i = Vector3_t<int>;
    using Vector3u = Vector3_t<unsigned int>;
    using Vector3f = Vector3_t<float>;
    using Vector3d = Vector3_t<double>;

    template<typename T>
    constexpr Vector3_t<T> cross(const Vector3_t<T>& a, const Vector3_t<T>& b)
    {
        return {
            a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0],
        };
    }
} // namespace orion
