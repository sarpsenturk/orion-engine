#pragma once

#include <cstddef>
#include <cstdint>

namespace orion
{
    constexpr std::uintptr_t align_address(std::uintptr_t addr, std::size_t alignment)
    {
        const auto mask = alignment - 1;
        return (addr + mask) & ~mask;
    }

    template<typename T>
    T* align_pointer(T* ptr, std::size_t alignment)
    {
        const auto addr         = reinterpret_cast<std::uintptr_t>(ptr);
        const auto addr_aligned = align(addr, alignment);
        return reinterpret_cast<T*>(addr_aligned);
    }
} // namespace orion
