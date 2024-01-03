#pragma once

#include <concepts>
#include <type_traits>

namespace orion
{
    template<typename T>
    concept LessThanComparable = requires(T a, T b) {
        {
            a < b
        } -> std::convertible_to<bool>;
    };

    template<typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;
} // namespace orion
