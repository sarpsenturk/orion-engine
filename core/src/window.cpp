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
        if (const auto* resize = event.get_if<WindowResize>()) {
            on_resize(resize);
        } else if (const auto* move = event.get_if<WindowMove>()) {
            on_move(move);
        } else if (event.get_if<WindowActivate>()) {
            on_activate();
        } else if (event.get_if<WindowDeactivate>()) {
            on_deactivate();
        }
        return event;
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
} // namespace orion
