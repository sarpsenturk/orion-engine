#pragma once

namespace orion
{
    enum class KeyCode {
        Space = 32,
        Apostrophe = 39,

        Comma = 44,
        Minus = 45,
        Period = 46,
        Slash = 47,

        K0 = 48,
        K1,
        K2,
        K3,
        K4,
        K5,
        K6,
        K7,
        K8,
        K9,

        Semicolon = 59,
        Equal = 61,

        A = 65,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        LeftBracket = 91,
        Backslash = 92,
        RightBracket = 93,
        GraveAccent = 96,

        World1 = 161,
        World2 = 162,

        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,

        RightArrow = 262,
        LeftArrow = 263,
        DownArrow = 264,
        UpArrow = 265,

        PageUp = 266,
        PageDown = 267,

        Home = 268,
        End = 269,

        CapsLock = 280,
        ScrollLock = 281,
        NumLock = 282,

        PrintScreen = 283,
        Pause = 284,

        F1 = 290,
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
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,

        Num0 = 320,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        NumDecimal = 330,
        NumDivide,
        NumMultiply,
        NumSubtract,
        NumAdd,
        NumEnter,
        NumEqual,

        LeftShift = 340,
        LeftControl = 341,
        LeftAlt = 342,
        LeftSuper = 343,
        RightShift = 344,
        RightControl = 345,
        RightAlt = 346,
        RightSuper = 347,
        Menu = 348,
    };

    enum class MouseButton {
        Button1 = 0,
        Button2,
        Button3,
        Button4,
        Button5,
        Button6,
        Button7,
        Button8,

        Left = Button1,
        Right = Button2,
        Middle = Button3,
    };

    struct Window;
    bool platform_input_key_pressed(const Window* window, KeyCode key);
    bool platform_input_key_released(const Window* window, KeyCode key);
    void platform_input_cursor_position(const Window* window, double* xpos, double* ypos);
    bool platform_input_mouse_button_pressed(const Window* window, MouseButton button);
    bool platform_input_mouse_button_released(const Window* window, MouseButton button);
    void platform_input_scroll_delta(const Window* window, double* xdelta, double* ydelta);
} // namespace orion