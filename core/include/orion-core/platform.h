#pragma once

namespace orion
{
    enum class Platform {
        Unknown,
        Windows,
        Linux
    };

    constexpr auto format_as(Platform platform) -> const char*
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
} // namespace orion
