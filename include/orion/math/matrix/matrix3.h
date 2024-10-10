#pragma once

#include "orion/math/matrix/matrix.h"

namespace orion
{
    template<typename T>
    using Matrix3_t = Matrix<T, 3, 3>;

    using Matrix3i = Matrix3_t<int>;
    using Matrix3u = Matrix3_t<unsigned int>;
    using Matrix3f = Matrix3_t<float>;
    using Matrix3d = Matrix3_t<double>;
} // namespace orion
