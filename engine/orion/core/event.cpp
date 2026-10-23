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

    std::string format_as(const OnKeyPress& event)
    {
        return fmt::format("OnKeyPress: window={}, key={}", fmt::ptr(event.window), static_cast<int>(event.key));
    }

    std::string format_as(const OnKeyRepeat& event)
    {
        return fmt::format("OnKeyRepeat: window={}, key={}", fmt::ptr(event.window), static_cast<int>(event.key));
    }

    std::string format_as(const OnKeyRelease& event)
    {
        return fmt::format("OnKeyRelease: window={}, key={}", fmt::ptr(event.window), static_cast<int>(event.key));
    }

    std::string format_as(const OnMouseButtonPress& event)
    {
        return fmt::format("OnMouseButtonPress: window={}, button={}", fmt::ptr(event.window), static_cast<int>(event.button));
    }

    std::string format_as(const OnMouseButtonRelease& event)
    {
        return fmt::format("OnMouseButtonRelease: window={}, button={}", fmt::ptr(event.window), static_cast<int>(event.button));
    }

    std::string format_as(const OnCursorMove& event)
    {
        return fmt::format("OnCursorMove: window={}, xpos={}, ypos={}", fmt::ptr(event.window), event.xpos, event.ypos);
    }

    std::string format_as(const Event& event)
    {
        return std::visit([](const auto& e) { return format_as(e); }, event);
    }
} // namespace orion
