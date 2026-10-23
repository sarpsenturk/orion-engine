#pragma once

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

    using Event = std::variant<
        OnWindowClose,
        OnWindowResize,
        OnWindowMove>;

    std::string format_as(const OnWindowClose& event);
    std::string format_as(const OnWindowResize& event);
    std::string format_as(const OnWindowMove& event);
    std::string format_as(const Event& event);
} // namespace orion
