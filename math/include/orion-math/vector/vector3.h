#pragma once

#include "vector.h"

#include <cstdint> // std::int32_t, std::uint32_t

namespace orion
{
    template<typename T>
    using Vector3_t = Vector<T, 3>;

    using Vector3_i = Vector3_t<std::int32_t>;
    using Vector3_u = Vector3_t<std::uint32_t>;
    using Vector3_f = Vector3_t<std::float_t>;
    using Vector3_d = Vector3_t<std::double_t>;

    template<typename T>
    [[nodiscard]] constexpr Vector3_t<T> vec3(const Vector<T, 2>& vector2, T z_value)
    {
        return {vector2[0], vector2[1], z_value};
    }
} // namespace orion
