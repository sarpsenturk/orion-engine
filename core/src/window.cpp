#include "orion-core/window.h"

#include "orion-utils/bits.h"

#ifndef ORION_WINDOW_LOG_LEVEL
    #define ORION_WINDOW_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include "orion-core/log.h"

#include <spdlog/spdlog.h>

namespace orion
{
    Window::Window(WindowDesc window_desc)
        : platform_window_(platform::create_window(this, window_desc), platform::destroy_window)
        , name_(std::move(window_desc.name))
        , position_(window_desc.position)
        , size_(window_desc.size)
    {
        // Set up window events handlers
        // Events are invoked from platform implementations
        on_move_ += [this](const auto& move) {
            position_ = move.position;
            set_bit(state_, move_bit);
        };
        on_move_end_ += [this](auto&) {
            clear_bit(state_, move_bit);
        };
        on_resize_ += [this](const auto& resize) {
            size_ = resize.size;
            set_bit(state_, resize_bit);
        };
        on_resize_end_ += [this](const auto&) {
            clear_bit(state_, resize_bit);
        };
        on_close_ += [this](const auto&) {
            set_bit(state_, close_bit);
        };
        on_maximize_ += [this](const auto&) {
            set_bit(state_, maximize_bit);
            clear_bit(state_, minimize_bit);
        };
        on_minimize_ += [this](const auto&) {
            set_bit(state_, minimize_bit);
            clear_bit(state_, maximize_bit);
        };
        on_restore_ += [this](const auto&) {
            clear_bit(state_, maximize_bit);
            clear_bit(state_, minimize_bit);
        };

        SPDLOG_LOGGER_DEBUG(logger(), "Window \"{}\" created", name_);
    }

    void Window::poll_events()
    {
        platform::update_window(platform_window_.get());
    }

    bool Window::should_close() const noexcept
    {
        return get_bit(state_, close_bit) != 0;
    }

    bool Window::is_resizing() const noexcept
    {
        return get_bit(state_, resize_bit) != 0;
    }

    bool Window::is_moving() const noexcept
    {
        return get_bit(state_, move_bit) != 0;
    }

    bool Window::is_maximized() const noexcept
    {
        return get_bit(state_, maximize_bit) != 0;
    }

    bool Window::is_minimized() const noexcept
    {
        return get_bit(state_, minimize_bit) != 0;
    }

    spdlog::logger* Window::logger()
    {
        static const auto window_logger = create_logger("orion-window", ORION_WINDOW_LOG_LEVEL);
        return window_logger.get();
    }

    namespace events
    {
        const char* format_as(const WindowClose&)
        {
            return "(event) OnWindowClose";
        }

        std::string format_as(const WindowMove& move)
        {
            return fmt::format("(event) OnWindowMove {{ position: {} }}", move.position);
        }

        std::string format_as(const WindowMoveEnd& move_end)
        {
            return fmt::format("(event) OnWindowMoveEnd {{ final_position: {} }}", move_end.position);
        }

        std::string format_as(const WindowResize& resize)
        {
            return fmt::format("(event) OnWindowResize {{ size: {} }}", resize.size);
        }

        std::string format_as(const WindowResizeEnd& resize_end)
        {
            return fmt::format("(event) OnWindowResizeEnd {{ final_size: {} }}", resize_end.size);
        }

        const char* format_as(const WindowMaximize&)
        {
            return "(event) OnWindowMaximize";
        }

        const char* format_as(const WindowMinimize&)
        {
            return "(event) OnWindowMinimize";
        }

        const char* format_as(const WindowRestore&)
        {
            return "(event) OnWindowRestore";
        }
    } // namespace events
} // namespace orion
