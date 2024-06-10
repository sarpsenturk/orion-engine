#pragma once

#include <cstdint>

namespace orion
{
    enum class Format : std::uint32_t {
        Undefined,
        R8_Unorm,
        B8G8R8A8_Srgb,
        R8G8B8A8_Unorm,
        R32_Uint,
        R32_Int,
        R32_Float,
        R32G32_Uint,
        R32G32_Int,
        R32G32_Float,
        R32G32B32_Uint,
        R32G32B32_Int,
        R32G32B32_Float,
        R32G32B32A32_Uint,
        R32G32B32A32_Int,
        R32G32B32A32_Float,
    };

    const char* format_as(Format format) noexcept;

    constexpr auto format_size(Format format) -> std::uint32_t
    {
        switch (format) {
            case Format::Undefined:
                break;
            case Format::B8G8R8A8_Srgb:
            case Format::R8G8B8A8_Unorm:
                return sizeof(uint8_t) * 4;
            case Format::R8_Unorm:
                return sizeof(std::uint8_t);
            case Format::R32_Uint:
                return sizeof(std::uint32_t);
            case Format::R32_Int:
                return sizeof(std::int32_t);
            case Format::R32_Float:
                return sizeof(float);
            case Format::R32G32_Uint:
                return sizeof(std::uint32_t) * 2;
            case Format::R32G32_Int:
                return sizeof(std::int32_t) * 2;
            case Format::R32G32_Float:
                return sizeof(float) * 2;
            case Format::R32G32B32_Uint:
                return sizeof(std::uint32_t) * 3;
            case Format::R32G32B32_Int:
                return sizeof(std::int32_t) * 3;
            case Format::R32G32B32_Float:
                return sizeof(float) * 3;
            case Format::R32G32B32A32_Uint:
                return sizeof(std::uint32_t) * 4;
            case Format::R32G32B32A32_Int:
                return sizeof(std::int32_t) * 4;
            case Format::R32G32B32A32_Float:
                return sizeof(float) * 4;
        }
        return UINT32_MAX;
    }
} // namespace orion
