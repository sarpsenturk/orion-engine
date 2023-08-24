#pragma once

#include <cstdint>
#include <fmt/core.h>
#include <functional>
#include <random>
#include <type_traits>

namespace orion
{
    template<typename Tag, typename KeyType = std::uint64_t>
    class Handle
    {
    public:
        using key_type = KeyType;

        static constexpr key_type min_handle = std::numeric_limits<key_type>::min();
        static constexpr key_type max_handle = std::numeric_limits<key_type>::max() - 1;
        static constexpr key_type invalid = std::numeric_limits<key_type>::max();

        static constexpr Handle invalid_handle() noexcept { return Handle{invalid}; };

        static Handle generate() noexcept
        {
            static thread_local std::random_device random_device;
            static thread_local std::mt19937_64 mt_19937_64(random_device());

            std::uniform_int_distribution<key_type> distribution(min_handle, max_handle);
            return Handle{distribution(mt_19937_64)};
        }

        constexpr Handle() noexcept = default;

        constexpr Handle(std::nullptr_t) noexcept
            : value_(invalid)
        {
        }

        constexpr explicit Handle(key_type value) noexcept
            : value_(value)
        {
        }

        [[nodiscard]] constexpr auto value() const noexcept { return value_; }

        [[nodiscard]] constexpr auto is_valid() const noexcept { return value_ != invalid; }

        [[nodiscard]] constexpr friend bool operator==(Handle, Handle) noexcept = default;

        [[nodiscard]] constexpr operator bool() const noexcept { return is_valid(); }

    private:
        key_type value_ = invalid;
    };

    template<typename Tag, typename ValueType>
    inline auto format_as(Handle<Tag, ValueType> handle)
    {
        return fmt::format("{}{{{}}}", Tag::string, handle.value());
    }
} // namespace orion

template<typename Tag, typename ValueType>
struct std::hash<orion::Handle<Tag, ValueType>> {
    using handle_type = orion::Handle<Tag, ValueType>;
    using value_type = typename handle_type::key_type;

    auto operator()(handle_type handle) const noexcept
    {
        return std::hash<value_type>{}(handle.value());
    }
};

// Simple helper macro to define a Handle
#define ORION_DEFINE_HANDLE(name, key_type)   \
    struct name##_tag {                       \
        static constexpr auto string = #name; \
    };                                        \
    using name = ::orion::Handle<name##_tag, key_type>
