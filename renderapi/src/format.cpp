#include "orion-renderapi/format.h"

namespace orion
{
    const char* format_as(Format format) noexcept
    {
        switch (format) {
            case Format::Undefined:
                return "Undefined";
            case Format::B8G8R8A8_Srgb:
                return "B8G8R8A8_Srgb";
            case Format::R8_Unorm:
                return "R8_Unorm";
            case Format::R32_Uint:
                return "R32_Uint";
            case Format::R32_Int:
                return "R32_Int";
            case Format::R32_Float:
                return "R32_Float";
            case Format::R32G32_Uint:
                return "R32G32_Uint";
            case Format::R32G32_Int:
                return "R32G32_Int";
            case Format::R32G32_Float:
                return "R32G32_Float";
            case Format::R32G32B32_Uint:
                return "R32G32B32_Uint";
            case Format::R32G32B32_Int:
                return "R32G32B32_Int";
            case Format::R32G32B32_Float:
                return "R32G32B32_Float";
            case Format::R32G32B32A32_Uint:
                return "R32G32B32A32_Uint";
            case Format::R32G32B32A32_Int:
                return "R32G32B32A32_Int";
            case Format::R32G32B32A32_Float:
                return "R32G32B32A32_Float";
            case Format::R8G8B8A8_Unorm:
                return "R8G8B8A8_Unorm";
        }
        return "Unknown format";
    }
} // namespace orion
