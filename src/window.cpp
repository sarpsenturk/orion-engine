#include "orion/window.h"

#include <fmt/format.h>
#include <fmt/std.h>

namespace orion
{
    std::string format_as(const OnWindowClose&)
    {
        return "WindowClose{}";
    }

    std::string format_as(const OnWindowResize& resize)
    {
        return fmt::format("WindowResize{{ width: {}, height: {} }}", resize.width, resize.height);
    }

    std::string format_as(const OnWindowMove& move)
    {
        return fmt::format("WindowMove{{ xpos: {}, ypos: {} }}", move.xpos, move.ypos);
    }

    std::string format_as(const WindowEvent& event)
    {
        return fmt::format("{}", event.data);
    }
} // namespace orion
