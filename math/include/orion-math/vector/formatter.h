#pragma once

#include "vector.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

// This is needed to resolve template ambiguity
// since Vector models std::range.
// See: https://github.com/fmtlib/fmt/issues/2667
template<typename T, std::size_t N>
struct fmt::is_range<orion::math::Vector<T, N>, char> : std::false_type {
};

template<typename T, std::size_t N>
struct fmt::formatter<orion::math::Vector<T, N>> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const orion::math::Vector<T, N>& vector, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "Vector{}({})", N, fmt::join(vector, ", "));
    }
};
