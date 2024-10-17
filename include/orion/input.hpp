#pragma once

#include <array>
#include <cstdint>

namespace orion
{
    enum class Keycode {
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

        Max,
    };

    const char* format_as(Keycode keycode);

    enum class MouseButton {
        Left,
        Right,
        Middle,
        Max,
    };

    const char* format_as(MouseButton mouse_button);

    class Keyboard
    {
    public:
        enum class KeyState : std::int8_t {
            Up = 0,
            Down,
        };

        [[nodiscard]] bool is_key_down(Keycode key) const;

        void set_key_down(Keycode key);
        void set_key_up(Keycode key);

    private:
        std::array<KeyState, static_cast<std::size_t>(Keycode::Max)> keys_ = {};
    };

    class Mouse
    {
    public:
        void set_button_down(MouseButton button);
        void set_button_up(MouseButton button);

        [[nodiscard]] bool is_button_down(MouseButton button) const;

    private:
        std::array<bool, static_cast<std::size_t>(MouseButton::Max)> buttons_ = {};
    };
} // namespace orion
