#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace orion
{
    enum class BufferUsageFlags : std::uint8_t {
        None = 0,
        VertexBuffer = 1,
        IndexBuffer = 2,
        ConstantBuffer = 4,
        StructuredBuffer = 8,
        TransferSrc = 16,
        TransferDst = 32,
    };

    // TODO: Allow declaring bit flags automatically
    constexpr BufferUsageFlags operator&(BufferUsageFlags lhs, BufferUsageFlags rhs)
    {
        return static_cast<BufferUsageFlags>(static_cast<std::underlying_type_t<BufferUsageFlags>>(lhs) & static_cast<std::underlying_type_t<BufferUsageFlags>>(rhs));
    }
    constexpr BufferUsageFlags operator|(BufferUsageFlags lhs, BufferUsageFlags rhs)
    {
        return static_cast<BufferUsageFlags>(static_cast<std::underlying_type_t<BufferUsageFlags>>(lhs) | static_cast<std::underlying_type_t<BufferUsageFlags>>(rhs));
    }

    struct BufferDesc {
        std::size_t size;
        BufferUsageFlags usage_flags;
        bool cpu_visible;
    };
} // namespace orion
