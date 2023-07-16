#pragma once

#include <cstdint>
#include <fmt/core.h>
#include <functional>
#include <random>
#include <type_traits>

namespace orion
{
    template<typename Tag>
    class Handle
    {
    public:
        using value_type = std::uint64_t;

        static constexpr value_type invalid = std::numeric_limits<value_type>::max();
        static constexpr value_type min_handle = 0;
        static constexpr value_type max_handle = std::numeric_limits<value_type>::max() - 1;

        static constexpr Handle invalid_handle() noexcept { return Handle{invalid}; };

        static Handle generate() noexcept
        {
            static thread_local std::random_device random_device;
            static thread_local std::mt19937_64 mt_19937_64(random_device());

            std::uniform_int_distribution<value_type> distribution(min_handle, max_handle);
            return Handle{distribution(mt_19937_64)};
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

    template<typename Tag>
    inline auto format_as(Handle<Tag> handle)
    {
        return fmt::format("{}{{{}}}", Tag::string, handle.value());
    }
} // namespace orion

template<typename Tag>
struct std::hash<orion::Handle<Tag>> {
    using handle_type = orion::Handle<Tag>;
    using value_type = typename handle_type::value_type;

    auto operator()(handle_type handle) const noexcept
    {
        return std::hash<value_type>{}(handle.value());
    }
};

// Simple helper macro to define a Handle
#define ORION_DEFINE_HANDLE(name)             \
    struct name##_tag {                       \
        static constexpr auto string = #name; \
    };                                        \
    using name = ::orion::Handle<name##_tag>\
