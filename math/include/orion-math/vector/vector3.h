#pragma once

#include "vector.h"

#include <cstdint> // std::int32_t, std::uint32_t

namespace orion::math
{
    template<typename T>
    using Vector3_t = Vector<T, 3>;

    using Vector3_i = Vector3_t<std::int32_t>;
    using Vector3_u = Vector3_t<std::uint32_t>;
    using Vector3_f = Vector3_t<float>;
    using Vector3_d = Vector3_t<double>;
    using Vector3 = Vector3_f;
} // namespace orion::math
