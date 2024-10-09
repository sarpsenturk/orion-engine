#pragma once

#include "orion/math/vector/vector.h"

namespace orion
{
    template<typename T>
    using Vector4_t = Vector<T, 4>;

    using Vector4i = Vector4_t<int>;
    using Vector4u = Vector4_t<unsigned int>;
    using Vector4f = Vector4_t<float>;
    using Vector4d = Vector4_t<double>;
} // namespace orion
