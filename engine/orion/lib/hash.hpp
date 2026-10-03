#pragma

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace orion
{
    constexpr std::uint64_t fnv1a_hash(std::string_view str)
    {
        static constexpr auto fnv_offset_basis = 0xcbf29ce484222325ull;
        static constexpr auto fnv_prime        = 0x00000100000001b3ull;

        std::uint64_t hash = fnv_offset_basis;
        for (const auto c : str) {
            hash ^= static_cast<std::uint64_t>(c);
            hash *= fnv_prime;
        }
        return hash;
    }
} // namespace orion
