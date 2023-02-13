#pragma once

#include <cstdint>      // std::uint8_t
#include <fmt/format.h> // fmt::formatter

namespace orion
{
    struct Version {
        std::uint8_t major;
        std::uint8_t minor;
        std::uint8_t patch;
    };
} // namespace orion

template<>
struct fmt::formatter<orion::Version> : formatter<string_view> {
    auto format(const orion::Version& version, auto& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}.{}.{}", version.major, version.minor, version.patch);
    }
};
