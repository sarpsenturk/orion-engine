#pragma once

#include "matrix.h"

namespace orion::math
{
    template<typename T>
    using Matrix3_t = Matrix<T, 3, 3>;

    using Matrix3_i = Matrix3_t<std::int32_t>;
    using Matrix3_u = Matrix3_t<std::uint32_t>;
    using Matrix3_f = Matrix3_t<float>;
    using Matrix3_d = Matrix3_t<double>;
    using Matrix3 = Matrix3_f;
} // namespace orion::math
