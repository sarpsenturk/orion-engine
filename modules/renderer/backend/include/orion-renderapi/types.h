#pragma once

#include <cstdint>                     // std::uint32_t
#include <orion-math/vector/vector2.h> // orion::math::Vector2
#include <span>                        // std::span
#include <string>                      // std::string

namespace orion
{
    enum class Format {
        B8G8R8A8_SRGB
    };

    constexpr auto to_string(Format format) noexcept -> const char*
    {
        switch (format) {
            case Format::B8G8R8A8_SRGB:
                return "B8G8R8A8_SRGB";
        }
        return "Unknown format";
    }

    enum class PhysicalDeviceType {
        Other,
        Integrated,
        Discrete,
        Virtual,
        CPU
    };

    constexpr auto to_string(PhysicalDeviceType type) noexcept
    {
        switch (type) {
            case PhysicalDeviceType::Other:
                break;
            case PhysicalDeviceType::Integrated:
                return "Integrated";
            case PhysicalDeviceType::Discrete:
                return "Discrete";
            case PhysicalDeviceType::Virtual:
                return "Virtual";
            case PhysicalDeviceType::CPU:
                return "CPU";
        }
        return "Other";
    }

    enum class ShaderType {
        Vertex,
        Fragment
    };
} // namespace orion
