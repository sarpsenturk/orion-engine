#pragma once

#include <orion-utils/enum.h>

namespace orion
{
    enum class Platform {
        Unknown,
        Windows,
        Linux
    };

    template<>
    struct enum_traits<Platform> {
        static constexpr bool bitwise_enabled = false;
        static auto to_string(Platform platform)
        {
            switch (platform) {
                case Platform::Unknown:
                    break;
                case Platform::Windows:
                    return "Windows";
                case Platform::Linux:
                    return "Linux";
            }
            return "Unknown";
        }
    };
} // namespace orion
