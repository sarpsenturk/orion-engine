#pragma once

#include <concepts> // std::floating_point

namespace orion::math
{
    template<std::floating_point Floating>
    inline constexpr Floating pi_v = static_cast<Floating>(3.141592653589793);

    inline constexpr auto pi = pi_v<double>;
} // namespace orion::math
