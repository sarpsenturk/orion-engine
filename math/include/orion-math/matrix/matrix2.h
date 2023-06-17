#pragma once

#include "matrix.h"

namespace orion::math
{
    template<typename T>
    using Matrix2_t = Matrix<T, 2, 2>;

    using Matrix2_i = Matrix2_t<std::int32_t>;
    using Matrix2_u = Matrix2_t<std::uint32_t>;
    using Matrix2_f = Matrix2_t<float>;
    using Matrix2_d = Matrix2_t<double>;
    using Matrix2 = Matrix2_f;
} // namespace orion::math
