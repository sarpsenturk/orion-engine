#pragma once

#include <array>
#include <cstddef>
#include <utility>

namespace orion
{
    template<std::size_t N, class Generator, std::size_t... I>
    constexpr auto make_array_impl(
        Generator&& generator,
        std::index_sequence<I...>)
    {
        using T = std::invoke_result_t<Generator&>;

        return std::array<T, N>{
            (static_cast<void>(I), std::invoke(generator))...};
    }

    // Generates std::array of size N using the provided generator function.
    // Does not require the result type of the generator to be default constructible.
    template<std::size_t N>
    constexpr auto make_array(auto&& generator)
    {
        return make_array_impl<N>(
            std::forward<decltype(generator)>(generator),
            std::make_index_sequence<N>{});
    }
} // namespace orion
