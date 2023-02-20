#pragma once

#include <cstdint>     // std::int64_t
#include <fmt/core.h>  // fmt::formatter
#include <functional>  // std::hash
#include <memory>      // std::shared_ptr
#include <random>      // std::random_device, std::mt19937_64, std::uniform_int_distribution
#include <type_traits> // std::is_signed

namespace orion
{
    template<typename Tag>
    class Handle
    {
    public:
        using value_type = std::uint64_t;

        static constexpr value_type invalid = std::numeric_limits<value_type>::max();
        static constexpr value_type min_handle = 0;
        static constexpr value_type max_handle = invalid - 1;

        static constexpr Handle invalid_handle() noexcept { return Handle{invalid}; };

        static Handle generate() noexcept
        {
            static thread_local std::random_device random_device;
            static thread_local std::mt19937_64 mt_19937_64(random_device());

            std::uniform_int_distribution<value_type> distribution(min_handle, max_handle);
            return distribution(mt_19937_64);
        }

        constexpr Handle() noexcept = default;

        constexpr explicit Handle(value_type value) noexcept
            : value_(value)
        {
        }

        [[nodiscard]] constexpr auto value() const noexcept { return value_; }

        [[nodiscard]] constexpr auto is_valid() const noexcept { return value_ != invalid; }

        [[nodiscard]] constexpr friend bool operator==(Handle, Handle) noexcept = default;

    private:
        value_type value_ = invalid;
    };
} // namespace orion

template<typename Tag>
struct std::hash<orion::Handle<Tag>> { // NOLINT(cert-dcl58-cpp)
    using handle_type = orion::Handle<Tag>;
    using value_type = typename handle_type::value_type;

    auto operator()(handle_type handle) const noexcept
    {
        return std::hash<value_type>{}(handle.value());
    }
};

template<typename Tag>
struct fmt::formatter<orion::Handle<Tag>> : formatter<typename orion::Handle<Tag>::value_type> {
    auto format(orion::Handle<Tag> handle, auto& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}", handle.value());
    }
};

// Simple helper macro to define a Handle
#define ORION_DEFINE_HANDLE(name)             \
    struct name##_tag;                        \
    using name = ::orion::Handle<name##_tag>; \
    using name##Ref = std::shared_ptr<name>;
