#pragma once

#include <cstdint>                     // std::uint32_t
#include <orion-math/vector/vector2.h> // orion::math::Vector2
#include <span>                        // std::span
#include <string>                      // std::string

namespace orion
{
    enum class Format {
        B8G8R8A8_SRGB,
        R32G32B32A32_FLOAT,
    };

    constexpr auto size_of(Format format) -> std::uint32_t
    {
        switch (format) {
            case Format::B8G8R8A8_SRGB:
                return 32;
            case Format::R32G32B32A32_FLOAT:
                return 128;
        }
        return UINT32_MAX;
    }

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

    enum class PrimitiveTopology {
        TriangleList
    };

    enum class FillMode {
        Solid,
        Wireframe,
    };

    enum class CullMode {
        None,
        Front,
        Back,
        FrontAndBack
    };

    enum class FrontFace {
        CounterClockWise,
        ClockWise
    };

    enum class InputRate {
        Vertex,
        Instance
    };
} // namespace orion
