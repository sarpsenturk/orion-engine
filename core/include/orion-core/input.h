#pragma once

#include "event.h"

#include "orion-math/vector/vector2.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

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

        std::string format_as(const KeyRelease& key_release);
        std::string format_as(const KeyPress& key_press);
        std::string format_as(const KeyRepeat& key_repeat);
    } // namespace events

    class Keyboard
    {
    public:
        static constexpr auto max_key = static_cast<std::size_t>(KeyCode::Max);

        Keyboard();

        [[nodiscard]] KeyState key_state(KeyCode key) const noexcept { return key_states_[get_key_index(key)]; }
        [[nodiscard]] bool key_down(KeyCode key) const noexcept { return key_state(key) == KeyState::Down; }
        [[nodiscard]] bool key_up(KeyCode key) const noexcept { return key_state(key) == KeyState::Up; }
        [[nodiscard]] bool key_pressed(KeyCode key) const noexcept;

        [[nodiscard]] auto& on_key_release() noexcept { return on_key_release_; }
        [[nodiscard]] auto& on_key_release() const noexcept { return on_key_release_; }
        [[nodiscard]] auto& on_key_press() noexcept { return on_key_press_; }
        [[nodiscard]] auto& on_key_press() const noexcept { return on_key_press_; }
        [[nodiscard]] auto& on_key_repeat() noexcept { return on_key_repeat_; }
        [[nodiscard]] auto& on_key_repeat() const noexcept { return on_key_repeat_; }

    private:
        [[nodiscard]] std::size_t get_key_index(KeyCode key) const noexcept;
        void set_state(KeyCode key, KeyState state) noexcept;

        std::array<KeyState, max_key> key_states_ = {};

        EventDispatcher<void(const events::KeyRelease&)> on_key_release_;
        EventDispatcher<void(const events::KeyPress&)> on_key_press_;
        EventDispatcher<void(const events::KeyRepeat&)> on_key_repeat_;
    };

    enum class MouseButton {
        Left,
        Right,
        Middle,
        X1,
        X2,
        Max
    };
    inline constexpr auto max_mouse_button = static_cast<std::size_t>(MouseButton::Max);

    using MousePosition = Vector2_i;

    inline constexpr auto x_button_max = 15;
    using MouseXButtonID = std::uint8_t;
    static_assert(x_button_max < std::numeric_limits<MouseXButtonID>::max());

    namespace events
    {
        struct MouseMove {
            MousePosition position;
        };

        struct MouseButtonDown {
            MouseButton button;
            MousePosition position;
        };

        struct MouseButtonUp {
            MouseButton button;
            MousePosition position;
        };

        struct MouseScroll {
            int delta;
            MousePosition position;
        };

        std::string format_as(const MouseMove& mouse_move);
        std::string format_as(const MouseButtonDown& mouse_button_down);
        std::string format_as(const MouseButtonUp& mouse_button_up);
        std::string format_as(const MouseScroll& mouse_scroll);
    } // namespace events

    class Mouse
    {
    public:
        Mouse();

        [[nodiscard]] bool button(MouseButton button) const noexcept { return button_states_[get_button_index(button)]; }
        [[nodiscard]] auto& position() const noexcept { return position_; }

        [[nodiscard]] auto& on_move() noexcept { return on_move_; }
        [[nodiscard]] auto& on_move() const noexcept { return on_move_; }
        [[nodiscard]] auto& on_button_down() noexcept { return on_button_down_; }
        [[nodiscard]] auto& on_button_down() const noexcept { return on_button_down_; }
        [[nodiscard]] auto& on_button_up() noexcept { return on_button_up_; }
        [[nodiscard]] auto& on_button_up() const noexcept { return on_button_up_; }
        [[nodiscard]] auto& on_scroll() const noexcept { return on_scroll_; }
        [[nodiscard]] auto& on_scroll() noexcept { return on_scroll_; }

    private:
        [[nodiscard]] std::size_t get_button_index(MouseButton button) const noexcept;
        void set_button_state(MouseButton button, bool is_down);
        void set_position(MousePosition position);

        std::array<bool, max_mouse_button> button_states_ = {};
        MousePosition position_ = {};

        EventDispatcher<void(const events::MouseMove&)> on_move_;
        EventDispatcher<void(const events::MouseButtonDown&)> on_button_down_;
        EventDispatcher<void(const events::MouseButtonUp&)> on_button_up_;
        EventDispatcher<void(const events::MouseScroll&)> on_scroll_;
    };

    std::string format_as(KeyCode keycode);
    const char* format_as(MouseButton mouse_button);
} // namespace orion
