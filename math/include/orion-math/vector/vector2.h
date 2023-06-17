#pragma once

#include "vector.h"

#include <cstdint> // std::int32_t, std::uint32_t

namespace orion::math
{
    template<typename T>
    using Vector2_t = Vector<T, 2>;

    using Vector2_i = Vector2_t<std::int32_t>;
    using Vector2_u = Vector2_t<std::uint32_t>;
    using Vector2_f = Vector2_t<float>;
    using Vector2_d = Vector2_t<double>;
    using Vector2 = Vector2_f;
} // namespace orion::math
