#include "orion/window.hpp"

#include <fmt/format.h>
#include <fmt/std.h>

namespace orion
{
    WindowEvent Window::poll_event()
    {
        auto event = poll_event_impl();
        if (const auto* keydown = event.as<OnKeyDown>()) {
            keyboard_.set_key_down(keydown->keycode);
        } else if (const auto* keyup = event.as<OnKeyUp>()) {
            keyboard_.set_key_up(keyup->keycode);
        } else if (const auto* mousebuttondown = event.as<OnMouseButtonDown>()) {
            mouse_.set_button_down(mousebuttondown->button);
        } else if (const auto* mousebuttonup = event.as<OnMouseButtonUp>()) {
            mouse_.set_button_up(mousebuttonup->button);
        }
        return event;
    }

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

    std::string format_as(const OnKeyDown& keydown)
    {
        return fmt::format("KeyDown{{ keycode: {} }}", keydown.keycode);
    }

    std::string format_as(const OnKeyUp& keyup)
    {
        return fmt::format("KeyUp{{ keycode: {} }}", keyup.keycode);
    }

    std::string format_as(const OnMouseMove& mousemove)
    {
        return fmt::format("MouseMove{{ xpos: {}, ypos: {} }}", mousemove.xpos, mousemove.ypos);
    }

    std::string format_as(const OnMouseButtonDown& mousebuttondown)
    {
        return fmt::format("MouseButtonDown{{ button: {}, xpos: {}, ypos: {} }}", mousebuttondown.button, mousebuttondown.xpos, mousebuttondown.ypos);
    }

    std::string format_as(const OnMouseButtonUp& mousebuttonup)
    {
        return fmt::format("MouseButtonUp{{ button: {}, xpos: {}, ypos: {} }}", mousebuttonup.button, mousebuttonup.xpos, mousebuttonup.ypos);
    }

    std::string format_as(const OnMouseScroll& mousescroll)
    {
        return fmt::format("MouseScroll{{ scroll: {}, xpos: {}, ypos: {} }}", mousescroll.scroll, mousescroll.xpos, mousescroll.ypos);
    }

    std::string format_as(const WindowEvent& event)
    {
        return fmt::format("{}", event.data);
    }
} // namespace orion
