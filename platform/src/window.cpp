#include "orion-platform/window.h"

#include <fmt/format.h>
#include <fmt/std.h>

namespace orion
{
    std::string format_as(const WindowClose& event)
    {
        return "WindowClose";
    }

    std::string format_as(const WindowResize& event)
    {
        return fmt::format("WindowResize {{ size: {}, type: {} }}", event.size, event.type);
    }

    std::string format_as(const WindowMove& event)
    {
        return fmt::format("WindowMove {{ position: {} }}", event.position);
    }

    std::string format_as(const WindowActivate& event)
    {
        return "WindowActivate";
    }

    std::string format_as(const WindowDeactivate& event)
    {
        return "WindowDeactivate";
    }

    std::string format_as(const KeyDown& event)
    {
        return fmt::format("KeyDown {{ key: {} }}", event.key);
    }

    std::string format_as(const KeyUp& event)
    {
        return fmt::format("KeyUp {{ key: {} }}", event.key);
    }

    std::string format_as(const KeyRepeat& event)
    {
        return fmt::format("KeyRepeat {{ key: {} }}", event.key);
    }

    std::string format_as(const MouseButtonDown& event)
    {
        return fmt::format("MouseButtonDown {{ button: {}, position: {} }}", event.button, event.position);
    }

    std::string format_as(const MouseButtonUp& event)
    {
        return fmt::format("MouseButtonUp {{ button: {}, position: {} }}", event.button, event.position);
    }

    std::string format_as(const MouseMove& event)
    {
        return fmt::format("MouseMove {{ position: {} }}", event.position);
    }

    std::string format_as(const MouseScroll& event)
    {
        return fmt::format("MouseScroll {{ delta: {}, position: {} }}", event.delta, event.position);
    }

    std::string format_as(const WindowEvent& event)
    {
        return fmt::format("(event) {}", event.data);
    }
} // namespace orion
