#pragma once

#include "orion/math/matrix/matrix.hpp"

namespace orion
{
    template<typename T>
    using Matrix2_t = Matrix<T, 2, 2>;

    using Matrix2i = Matrix2_t<int>;
    using Matrix2u = Matrix2_t<unsigned int>;
    using Matrix2f = Matrix2_t<float>;
    using Matrix2d = Matrix2_t<double>;
} // namespace orion
