#pragma once

#include <cstdint>                     // std::uint32_t
#include <orion-math/vector/vector2.h> // orion::math::Vector2
#include <orion-utils/enum.h>          // orion::enum_traits
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
                return sizeof(uint8_t) * 4;
            case Format::R32G32B32A32_FLOAT:
                return sizeof(float) * 4;
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

    enum class GPUBufferUsageFlags : std::uint8_t {
        VertexBuffer = 1u << 0u,
        IndexBuffer = 1u << 1u,
        TransferSrc = 1u << 2u,
        TransferDst = 1u << 3u
    };

    template<>
    struct enum_traits<GPUBufferUsageFlags> : default_enum_traits<GPUBufferUsageFlags> {
        static constexpr bool bitwise_enabled = true;
    };

    struct Viewport {
        math::Vector2_f position;
        math::Vector2_f size;
    };

    enum class CommandQueueType {
        Graphics,
        Transfer,
        Compute,
        Any
    };

    enum class CommandType {
        BufferCopy,
        BeginFrame,
        EndFrame,
        Draw,
        DrawIndexed,
    };
} // namespace orion
