#pragma once

#include <cstdint>
#include <fmt/core.h>
namespace orion
{
    struct Version {
        std::uint8_t major;
        std::uint8_t minor;
        std::uint8_t patch;
    };

    inline constexpr auto engine_version = Version{ORION_VERSION_MAJOR, ORION_VERSION_MAJOR, ORION_VERSION_PATCH};

    inline auto format_as(const Version& version)
    {
        return fmt::format("{}.{}.{}", version.major, version.minor, version.patch);
    }
} // namespace orion
