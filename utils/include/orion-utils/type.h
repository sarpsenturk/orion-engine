#pragma once

#include <concepts>
#include <type_traits>

namespace orion
{
    template<typename T, typename... Ts>
    concept any_of = (... || std::same_as<T, Ts>);

    template<typename T, typename... Ts>
    concept all_of = (... && std::same_as<T, Ts>);

    template<typename... Ts>
    concept not_empty = (sizeof...(Ts) > 0);

    template<typename Enum>
    [[nodiscard]] constexpr auto to_underlying(Enum value) noexcept -> std::underlying_type_t<Enum>
    {
        return static_cast<std::underlying_type_t<Enum>>(value);
    }
} // namespace orion
