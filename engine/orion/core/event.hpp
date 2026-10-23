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

    using Event = std::variant<
        OnWindowClose,
        OnWindowResize,
        OnWindowMove,
        OnKeyPress,
        OnKeyRepeat,
        OnKeyRelease,
        >;

    std::string format_as(const OnWindowClose& event);
    std::string format_as(const OnWindowResize& event);
    std::string format_as(const OnWindowMove& event);
    std::string format_as(const OnKeyPress& event);
    std::string format_as(const OnKeyRepeat& event);
    std::string format_as(const OnKeyRelease& event);
    std::string format_as(const Event& event);
} // namespace orion
