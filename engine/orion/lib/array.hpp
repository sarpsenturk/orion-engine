#pragma once

#include <array>
#include <cstddef>
#include <utility>

namespace orion
{
    template<std::size_t N>
    constexpr auto make_array(auto&& generator)
    {
        std::array<std::remove_cvref_t<decltype(generator())>, N> arr{};
        for (std::size_t i = 0; i < N; ++i) {
            arr[i] = generator();
        }
        return arr;
    }
} // namespace orion
