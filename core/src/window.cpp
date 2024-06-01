#include "orion-core/window.h"

#include "orion-utils/bits.h"
#include "orion-utils/type.h"

#ifndef ORION_WINDOW_LOG_LEVEL
    #define ORION_WINDOW_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"

#include <spdlog/spdlog.h>

namespace orion
{
    Window::Window(WindowDesc window_desc)
        : platform_window_(platform::create_window(window_desc), &platform::destroy_window)
        , name_(std::move(window_desc.name))
        , position_(window_desc.position)
        , size_(window_desc.size)
    {
    }

    WindowEvent Window::poll_event()
    {
        auto event = platform::poll_event_window(platform_window_.get());
        on_event(event);
        return event;
    }

    void Window::on_event(const WindowEvent& event)
    {
        if (const auto* resize = event.get_if<WindowResize>()) {
            on_resize(resize);
        } else if (const auto* move = event.get_if<WindowMove>()) {
            on_move(move);
        } else if (event.get_if<WindowActivate>()) {
            on_activate();
        } else if (event.get_if<WindowDeactivate>()) {
            on_deactivate();
        } else if (const auto* keydown = event.get_if<KeyDown>()) {
            on_keydown(keydown);
        } else if (const auto* keyup = event.get_if<KeyUp>()) {
            on_keyup(keyup);
        } else if (const auto* mousedown = event.get_if<MouseButtonDown>()) {
            on_mousedown(mousedown);
        } else if (const auto* mouseup = event.get_if<MouseButtonUp>()) {
            on_mouseup(mouseup);
        } else if (const auto* mousemove = event.get_if<MouseMove>()) {
            on_mousemove(mousemove);
        }
    }

    bool Window::is_maximized() const noexcept
    {
        return (state_ & window_maximized) != 0;
    }

    bool Window::is_minimized() const noexcept
    {
        return (state_ & window_minimized) != 0;
    }

    bool Window::has_focus() const noexcept
    {
        return (state_ & window_focused) != 0;
    }

    spdlog::logger* Window::logger()
    {
        static const auto window_logger = create_logger("orion-window", ORION_WINDOW_LOG_LEVEL);
        return window_logger.get();
    }

    void Window::on_resize(const WindowResize* resize)
    {
        size_ = resize->size;
        state_ &= ~0xFD;
        state_ |= resize->type;
    }

    void Window::on_move(const WindowMove* move)
    {
        position_ = move->position;
    }

    void Window::on_activate()
    {
        state_ |= window_focused;
    }

    void Window::on_deactivate()
    {
        state_ &= ~(window_focused);
    }

    void Window::on_keydown(const KeyDown* keydown)
    {
        keyboard_.set(keydown->key);
    }

    void Window::on_keyup(const KeyUp* keyup)
    {
        keyboard_.clear(keyup->key);
    }

    void Window::on_mousedown(const MouseButtonDown* mousedown)
    {
        mouse_.set(mousedown->button);
    }

    void Window::on_mouseup(const MouseButtonUp* mouseup)
    {
        mouse_.clear(mouseup->button);
    }

    void Window::on_mousemove(const MouseMove* mousemove)
    {
        mouse_.set_position(mousemove->position);
    }
} // namespace orion
