#pragma once

#include "orion/math/matrix/matrix4.hpp"

#include "orion/math/vector/vector3.hpp"

namespace orion
{
    template<typename T = float>
    Matrix4_t<T> translate(T x, T y, T z)
    {
        return {
            T{1}, T{0}, T{0}, T{0},
            T{0}, T{1}, T{0}, T{0},
            T{0}, T{0}, T{1}, T{0},
            x, y, z, T{0}};
    }

    template<typename T = float>
    Matrix4_t<T> translate(const Vector3_t<T>& vector)
    {
        return translate(vector.x, vector.y, vector.z);
    }

    template<typename T = float>
    Matrix4_t<T> scale(T x, T y, T z)
    {
        return {
            x, T{0}, T{0}, T{0},
            T{0}, y, T{0}, T{0},
            T{0}, T{0}, z, T{0},
            T{0}, T{0}, T{0}, T{1}};
    }

    template<typename T = float>
    Matrix4_t<T> scale(const Vector3_t<T>& vector)
    {
        return scale(vector.x, vector.y, vector.z);
    }
} // namespace orion
