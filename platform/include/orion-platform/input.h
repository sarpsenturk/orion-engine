#pragma once

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

    enum class MouseButton {
        Left,
        Right,
        Middle,
        X1,
        X2,
        Max
    };

    using MousePosition = Vector2_i;

    std::string format_as(KeyCode key_code);
    const char* format_as(MouseButton mouse_button);
} // namespace orion
