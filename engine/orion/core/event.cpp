#include "event.hpp"

#include <fmt/format.h>

namespace orion
{
    std::string format_as(const OnWindowClose& event)
    {
        return fmt::format("OnWindowClose: window={}", fmt::ptr(event.window));
    }

    std::string format_as(const OnWindowResize& event)
    {
        return fmt::format("OnWindowResize: window={}, width={}, height={}",
                           fmt::ptr(event.window), event.width, event.height);
    }

    std::string format_as(const OnWindowMove& event)
    {
        return fmt::format("OnWindowMove: window={}, x={}, y={}",
                           fmt::ptr(event.window), event.x, event.y);
    }

    std::string format_as(const Event& event)
    {
        return std::visit([](const auto& e) { return format_as(e); }, event);
    }
} // namespace orion
