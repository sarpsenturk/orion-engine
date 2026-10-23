#pragma once

#include "input.hpp"

#include <string>
#include <variant>

namespace orion
{
    // Forward declarations
    class Window;

    struct OnWindowClose {
        Window* window;
    };

    struct OnWindowResize {
        Window* window;
        int width;
        int height;
    };

    struct OnWindowMove {
        Window* window;
        int x;
        int y;
    };

    struct OnKeyPress {
        Window* window;
        Keycode key;
    };

    struct OnKeyRepeat {
        Window* window;
        Keycode key;
    };

    struct OnKeyRelease {
        Window* window;
        Keycode key;
    };

    struct OnMouseButtonPress {
        Window* window;
        MouseButton button;
    };

    struct OnMouseButtonRelease {
        Window* window;
        MouseButton button;
    };

    struct OnCursorMove {
        Window* window;
        double xpos;
        double ypos;
    };

    struct OnScoll {
        Window* window;
        double xoffset;
        double yoffset;
    };

    using Event = std::variant<
        OnWindowClose,
        OnWindowResize,
        OnWindowMove,
        OnKeyPress,
        OnKeyRepeat,
        OnKeyRelease,
        OnMouseButtonPress,
        OnMouseButtonRelease,
        OnCursorMove,
        OnScoll>;

    std::string format_as(const OnWindowClose& event);
    std::string format_as(const OnWindowResize& event);
    std::string format_as(const OnWindowMove& event);
    std::string format_as(const OnKeyPress& event);
    std::string format_as(const OnKeyRepeat& event);
    std::string format_as(const OnKeyRelease& event);
    std::string format_as(const OnMouseButtonPress& event);
    std::string format_as(const OnMouseButtonRelease& event);
    std::string format_as(const OnCursorMove& event);
    std::string format_as(const OnScoll& event);
    std::string format_as(const Event& event);
} // namespace orion
