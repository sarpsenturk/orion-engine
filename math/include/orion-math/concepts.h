#pragma once

#include <type_traits> // std::is_arithmetic

namespace orion
{
    template<typename T>
    concept arithmetic = std::is_arithmetic_v<T>;
}
