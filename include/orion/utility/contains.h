#ifndef ORION_ENGINE_CONTAINS_H
#define ORION_ENGINE_CONTAINS_H

#include <type_traits>

namespace orion::types
{
    template<typename T, typename... Ts>
    struct contains : std::disjunction<std::is_same<T, Ts>...> {
    };

    template<typename T, typename... Ts>
    constexpr bool contains_v = contains<T, Ts...>::value;
} // namespace orion::detail

#endif // ORION_ENGINE_CONTAINS_H
