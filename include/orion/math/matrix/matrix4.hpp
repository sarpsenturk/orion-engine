#pragma once

#include "orion/math/matrix/matrix.hpp"

namespace orion
{
    template<typename T>
    using Matrix4_t = Matrix<T, 4, 4>;

    using Matrix4i = Matrix4_t<int>;
    using Matrix4u = Matrix4_t<unsigned int>;
    using Matrix4f = Matrix4_t<float>;
    using Matrix4d = Matrix4_t<double>;
} // namespace orion
