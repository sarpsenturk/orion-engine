#pragma once

#include "orion/math/vector/vector.hpp"

namespace orion
{
    template<typename T>
    using Vector2_t = Vector<T, 2>;

    using Vector2i = Vector2_t<int>;
    using Vector2u = Vector2_t<unsigned int>;
    using Vector2f = Vector2_t<float>;
    using Vector2d = Vector2_t<double>;
} // namespace orion
