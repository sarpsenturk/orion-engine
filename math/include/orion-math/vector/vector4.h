#pragma once

#include "vector.h"

#include <cstdint> // std::int32_t, std::uint32_t

namespace orion::math
{
    template<typename T>
    using Vector4_t = Vector<T, 4>;

    using Vector4_i = Vector4_t<std::int32_t>;
    using Vector4_u = Vector4_t<std::uint32_t>;
    using Vector4_f = Vector4_t<float>;
    using Vector4_d = Vector4_t<double>;
    using Vector4 = Vector4_f;
} // namespace orion::math
