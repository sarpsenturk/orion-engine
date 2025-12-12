#pragma once

#include <cstdint>

namespace orion
{
    template<typename Tag>
    struct RHIHandle {
        std::uint64_t value;

        static constexpr RHIHandle invalid() noexcept { return RHIHandle{0}; }
        constexpr bool is_valid() const noexcept { return value != 0; }

        friend constexpr bool operator==(const RHIHandle&, const RHIHandle&) = default;
        explicit operator std::uint64_t() const noexcept { return value; }
    };

    using RHISwapchain = RHIHandle<struct RHISwapchain_Tag>;
    using RHIPipeline = RHIHandle<struct RHIPipeline_Tag>;
    using RHISemaphore = RHIHandle<struct RHISemaphore_Tag>;
    using RHIFence = RHIHandle<struct RHIFence_Tag>;
    using RHIImage = RHIHandle<struct RHIImage_Tag>;
    using RHIImageView = RHIHandle<struct RHIImageView_Tag>;
} // namespace orion
