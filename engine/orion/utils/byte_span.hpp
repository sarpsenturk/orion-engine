#pragma once

#include <cstddef>
#include <type_traits>

namespace orion::gpu
{
    struct ByteSpan {
        const std::byte* data = nullptr;
        std::size_t size      = 0z;

        constexpr ByteSpan() = default;
        constexpr ByteSpan(const void* data, std::size_t size) noexcept
            : data(static_cast<const std::byte*>(data))
            , size(size)
        {
        }
        template<typename T>
            requires std::is_trivially_copyable_v<T>
        ByteSpan(const T& obj) noexcept
            : data(reinterpret_cast<const std::byte*>(&obj))
            , size(sizeof(T))
        {
        }

        [[nodiscard]] constexpr bool empty() const noexcept { return size == 0z; }
    };
} // namespace orion::gpu
