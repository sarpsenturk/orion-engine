#pragma once

#include <array>
#include <concepts>
#include <type_traits>

// make_array:
//  These functions are for when you want to create a fixed size array
//  whose size depends on a constexpr variable.
//  Ex:
//      constexpr auto count = 3;
//      const auto arr = std::array{generate(), generate(), generate()};
//  The problem here is even thouh 'count' is known at compile time, if it was to change
//  the code initializing the array above would become incorrect.
//  It would be possible to use functions like std::fill, std::generate or others, however
//  this requires the contained objects to be, copiable and/or default constructable.
//  make_array compiles to the equivalent of std::array{get_data(0), get_data(1), ..., get_data(n)}
//  which creates the array objects 'in place', without copies or default constructions

namespace orion
{
    template<auto N, typename T>
    constexpr std::array<T, N> make_array(T&& value)
    {
        using index_type = decltype(N);
        return [&]<index_type... Indices>(std::integer_sequence<index_type, Indices...>) {
            return std::array{(Indices, std::forward<T>(value))...};
        }(std::make_integer_sequence<index_type, N>());
    }

    template<auto N, std::invocable G>
    constexpr std::array<std::invoke_result_t<G>, N> make_array(G&& generator)
    {
        using index_type = decltype(N);
        return [&]<index_type... Indices>(std::integer_sequence<index_type, Indices...>) {
            return std::array{(Indices, std::invoke(std::forward<G>(generator)))...};
        }(std::make_integer_sequence<index_type, N>());
    }

    template<auto N, std::invocable<decltype(N)> G>
    constexpr std::array<std::invoke_result_t<G, decltype(N)>, N> make_array(G&& generator)
    {
        using index_type = decltype(N);
        return [&]<index_type... Indices>(std::integer_sequence<index_type, Indices...>) {
            return std::array{std::invoke(std::forward<G>(generator), Indices)...};
        }(std::make_integer_sequence<index_type, N>());
    }
} // namespace orion
