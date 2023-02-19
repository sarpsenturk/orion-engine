#pragma once

#include <cstdint>     // std::int64_t
#include <functional>  // std::hash
#include <type_traits> // std::is_signed

namespace orion
{
    template<typename Tag>
    class Handle
    {
    public:
        using value_type = std::int64_t;
        static_assert(std::is_signed_v<value_type>, "value_type must be a signed type");

        static constexpr value_type invalid = -1;

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

// Simple helper macro to define a Handle
#define ORION_DEFINE_HANDLE(name) \
    struct name##_tag;            \
    using name = ::orion::Handle<name##_tag>
