#ifndef ORION_ENGINE_INDEX_OF_H
#define ORION_ENGINE_INDEX_OF_H

#include <type_traits>

namespace orion::types
{
    template<typename T, typename... Ts>
    struct index_of {
    };
    template<typename T, typename... Ts>
    struct index_of<T, T, Ts...> : std::integral_constant<std::size_t, 0> {
    };
    template<typename T, typename U, typename... Ts>
    struct index_of<T, U, Ts...>
        : std::integral_constant<std::size_t, 1 + index_of<T, Ts...>::value> {
    };

    template<typename T, typename... Ts>
    constexpr std::size_t index_of_v = index_of<T, Ts...>::value;
} // namespace orion::detail

#endif // ORION_ENGINE_INDEX_OF_H
