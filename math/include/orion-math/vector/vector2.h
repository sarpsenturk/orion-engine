#pragma once

#include "vector.h"

#include <cstdint>

namespace orion
{
    template<typename T>
    using Vector2_t = Vector<T, 2>;

    using Vector2_i = Vector2_t<std::int32_t>;
    using Vector2_u = Vector2_t<std::uint32_t>;
    using Vector2_f = Vector2_t<std::float_t>;
    using Vector2_d = Vector2_t<std::double_t>;

    template<typename T>
    [[nodiscard]] constexpr Vector2_t<T> vec2(T x, T y)
    {
        return {x, y};
    }
} // namespace orion
