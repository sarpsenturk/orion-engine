#pragma once

#include "vector.h"

#include <cstdint> // std::int32_t, std::uint32_t

namespace orion
{
    template<typename T>
    using Vector4_t = Vector<T, 4>;

    using Vector4_i = Vector4_t<std::int32_t>;
    using Vector4_u = Vector4_t<std::uint32_t>;
    using Vector4_f = Vector4_t<std::float_t>;
    using Vector4_d = Vector4_t<std::double_t>;

    template<typename T>
    [[nodiscard]] constexpr Vector4_t<T> vec4(const Vector<T, 2>& vector2, T z_value, T w_value)
    {
        return {vector2[0], vector2[1], z_value, w_value};
    }

    template<typename T>
    [[nodiscard]] constexpr Vector4_t<T> vec4(const Vector<T, 3>& vector3, T w_value)
    {
        return {vector3[0], vector3[1], vector3[2], w_value};
    }
} // namespace orion
