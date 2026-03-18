#include "orion/window.hpp"

#include <fmt/format.h>

namespace orion
{
    std::string format_as(const OnWindowClose&)
    {
        return "OnWindowClose";
    }

    std::string format_as(const OnWindowResize& event)
    {
        return fmt::format("OnWindowResize{{ width: {}, height: {} }}", event.width, event.height);
    }

    std::string format_as(const OnWindowMove& event)
    {
        return fmt::format("OnWindowMove{{ xpos: {}, ypos: {} }}", event.xpos, event.ypos);
    }
} // namespace orion
