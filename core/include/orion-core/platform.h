#pragma once

#include <cstdint>

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

    struct ProcessorFeatureSet {
        bool sse    : 1;
        bool sse2   : 1;
        bool sse3   : 1;
        bool sse4_1 : 1;
        bool sse4_2 : 1;
        bool avx    : 1;
        bool avx2   : 1;
        bool avx512 : 1;
    };

    // Implemented in platform files

    extern ProcessorFeatureSet get_processor_features();
} // namespace orion
