#pragma once

#include "matrix.h"

namespace orion::math
{
    template<typename T>
    using Matrix4_t = Matrix<T, 4, 4>;

    using Matrix4_i = Matrix4_t<std::int32_t>;
    using Matrix4_u = Matrix4_t<std::uint32_t>;
    using Matrix4_f = Matrix4_t<float>;
    using Matrix4_d = Matrix4_t<double>;
    using Matrix4 = Matrix4_f;
} // namespace orion::math
