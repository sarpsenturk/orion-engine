#pragma once

#include "orion-math/vector/vector2.h"

#include <bitset>
#include <cstddef>
#include <cstdint>

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
    };

    class Keyboard
    {
    public:
        void set(KeyCode key);
        void clear(KeyCode key);

        [[nodiscard]] bool is_down(KeyCode key) const;

        [[nodiscard]] bool any() const noexcept;
        [[nodiscard]] bool none() const noexcept;

    private:
        // std::bitset seems to use std::size_t for internal storage
        // on all major compilers meaning it will use 24 bytes / 192 bits
        // even if we request 177, which is the max KeyCode value
        std::bitset<192> keys_;
    };

    enum class MouseButton {
        Unknown = -1,
        Left,
        Right,
        Middle,
        X1,
        X2,
    };

    using MousePosition = Vector2_i;

    class Mouse
    {
    public:
        void set(MouseButton button);
        void clear(MouseButton button);

        [[nodiscard]] bool is_down(MouseButton button) const;

        [[nodiscard]] bool any() const noexcept;
        [[nodiscard]] bool none() const noexcept;

        void set_position(MousePosition position);
        [[nodiscard]] auto& position() const { return position_; }

    private:
        MousePosition position_;
        std::uint8_t buttons_ = 0;
    };

    const char* format_as(KeyCode key_code);
    const char* format_as(MouseButton mouse_button);
} // namespace orion
