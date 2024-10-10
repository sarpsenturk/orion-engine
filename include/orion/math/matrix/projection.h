#pragma once

#include "orion/math/matrix/matrix.h"

namespace orion
{
    template<typename T = float>
    constexpr Matrix<T, 4, 4> orthographic_rh(T left, T right, T bottom, T top, T near, T far)
    {
        auto projection = Matrix<T, 4, 4>::identity();
        projection(0, 0) = T{2} / (right - left);
        projection(1, 1) = T{2} / (top - bottom);
        projection(2, 2) = -T{1} / (far - near);
        projection(3, 0) = -(right + left) / (right - left);
        projection(3, 1) = -(top + bottom) / (top - bottom);
        projection(3, 2) = -near / (far - near);
        return projection;
    }
} // namespace orion
