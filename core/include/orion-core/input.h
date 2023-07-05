#pragma once

#include "event.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include <fmt/format.h>

namespace orion
{
    enum class KeyCode {
        Unknown = -1,
        // Start ASCII characters

        Backspace = 8,
        Tab = 9,
        Enter = 13,
        Escape = 27,
        Space = 32,
        Quote = 39,
        Comma = 44,
        Minus = 45,
        Period = 46,
        Slash = 47,

        Alpha0 = 48,
        Alpha1,
        Alpha2,
        Alpha3,
        Alpha4,
        Alpha5,
        Alpha6,
        Alpha7,
        Alpha8,
        Alpha9,

        Semicolon = 59,
        Equal = 61,

        KeyA = 65,
        KeyB,
        KeyC,
        KeyD,
        KeyE,
        KeyF,
        KeyG,
        KeyH,
        KeyI,
        KeyJ,
        KeyK,
        KeyL,
        KeyM,
        KeyN,
        KeyO,
        KeyP,
        KeyQ,
        KeyR,
        KeyS,
        KeyT,
        KeyU,
        KeyV,
        KeyW,
        KeyX,
        KeyY,
        KeyZ,

        LeftBracket = 91,
        Backslash = 92,
        RightBracket = 93,
        Backtick = 96,
        Delete = 127,
        // End ASCII characters

        Insert,
        Home,
        End,
        PageUp,
        PageDown,

        PrintScreen,
        ScrollLock,
        Pause,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        FMax,

        Caps,
        Shift,
        Control,
        Alt,
        Command,

        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        NumDivide,
        NumMultiply,
        NumMinus,
        NumPlus,
        NumEnter,
        NumPeriod,
        NumLock,

        LeftArrow,
        UpArrow,
        RightArrow,
        DownArrow,

        Max
    };

    std::string to_string(KeyCode keycode) noexcept;

    constexpr bool is_numeric_key(KeyCode keycode) noexcept
    {
        return (keycode >= KeyCode::Alpha0) && (keycode <= KeyCode::Alpha9);
    }
    constexpr bool is_char_key(KeyCode keycode) noexcept
    {
        return (keycode >= KeyCode::KeyA) && (keycode <= KeyCode::KeyZ);
    }
    constexpr bool is_numpad_key(KeyCode keycode) noexcept
    {
        return (keycode >= KeyCode::Num0) && (keycode <= KeyCode::Num9);
    }
    constexpr bool is_fn_key(KeyCode keycode) noexcept
    {
        return (keycode >= KeyCode::F1) && (keycode < KeyCode::FMax);
    }

    enum class KeyState : std::uint8_t {
        Up,
        Down,
        Repeated
    };

    namespace events
    {
        struct KeyRelease {
            KeyCode key;
        };

        struct KeyPress {
            KeyCode key;
        };

        struct KeyRepeat {
            KeyCode key;
        };
    } // namespace events

    class Keyboard
    {
    public:
        static constexpr auto max_key = static_cast<std::size_t>(KeyCode::Max);

        Keyboard();

        [[nodiscard]] KeyState key_state(KeyCode key) const noexcept { return key_states_[static_cast<std::size_t>(key)]; }
        [[nodiscard]] bool key_down(KeyCode key) const noexcept { return key_state(key) == KeyState::Down; }

        [[nodiscard]] auto& on_key_release() noexcept { return on_key_release_; }
        [[nodiscard]] auto& on_key_release() const noexcept { return on_key_release_; }
        [[nodiscard]] auto& on_key_press() noexcept { return on_key_press_; }
        [[nodiscard]] auto& on_key_press() const noexcept { return on_key_press_; }
        [[nodiscard]] auto& on_key_repeat() noexcept { return on_key_repeat_; }
        [[nodiscard]] auto& on_key_repeat() const noexcept { return on_key_repeat_; }

    private:
        void set_state(KeyCode key, KeyState state) noexcept;

        std::array<KeyState, max_key> key_states_;

        EventDispatcher<void(const events::KeyRelease&)> on_key_release_;
        EventDispatcher<void(const events::KeyPress&)> on_key_press_;
        EventDispatcher<void(const events::KeyRepeat&)> on_key_repeat_;
    };
} // namespace orion

template<>
struct fmt::formatter<orion::events::KeyPress> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::KeyPress& key_press, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnKeyPress {{ key: {} }}", orion::to_string(key_press.key));
    }
};

template<>
struct fmt::formatter<orion::events::KeyRelease> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::KeyRelease& key_release, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnKeyRelease {{ key: {} }}", orion::to_string(key_release.key));
    }
};

template<>
struct fmt::formatter<orion::events::KeyRepeat> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::KeyRepeat& key_repeat, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnKeyRepeat {{ key: {} }}", orion::to_string(key_repeat.key));
    }
};
