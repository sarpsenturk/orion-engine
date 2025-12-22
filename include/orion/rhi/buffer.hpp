#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace orion
{
    enum class RHIBufferUsageFlags : std::uint8_t {
        None = 0,
        TransferSrc = 1,
        TransferDst = 2,
        VertexBuffer = 4,
        IndexBuffer = 8,
        ConstantBuffer = 16,
    };
    inline constexpr RHIBufferUsageFlags operator&(RHIBufferUsageFlags lhs, RHIBufferUsageFlags rhs)
    {
        return static_cast<RHIBufferUsageFlags>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
    }

    struct RHIBufferDesc {
        std::size_t size;
        RHIBufferUsageFlags usage;
        std::span<const std::byte> initial_data;
    };
} // namespace orion
