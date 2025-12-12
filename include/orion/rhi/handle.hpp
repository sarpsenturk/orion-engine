#pragma once

#include <cstdint>

namespace orion
{
    template<typename Tag>
    struct RHIHandle {
        std::uint64_t value;

        static constexpr RHIHandle invalid() noexcept { return RHIHandle{0}; }
        constexpr bool is_valid() const noexcept { return value == 0; }

        friend constexpr bool operator==(const RHIHandle&, const RHIHandle&) = default;
        explicit operator std::uint64_t() const noexcept { return value; }
    };

    using RHIPipeline = RHIHandle<struct RHIPipeline_Tag>;
} // namespace orion
