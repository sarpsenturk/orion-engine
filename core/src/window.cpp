#include "orion-core/window.h"

#ifndef ORION_WINDOW_LOG_LEVEL
    #define ORION_WINDOW_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include "orion-core/log.h"

#include <spdlog/spdlog.h>

namespace orion
{
    Window::Window(WindowCreateInfo window_create_info)
        : platform_window_(platform::create_window(this, window_create_info), platform::destroy_window)
        , name_(std::move(window_create_info.name))
        , position_(window_create_info.position)
        , size_(window_create_info.size)
    {
        // Set up window events handlers
        // Events are invoked from platform implementations
        on_move_ += [this](const auto& move) {
            position_ = move.position;
            is_moving_ = true;
        };
        on_move_end_ += [this](auto&) {
            is_moving_ = false;
        };
        on_resize_ += [this](const auto& resize) {
            size_ = resize.size;
            is_resizing_ = true;
        };
        on_resize_end_ += [this](const auto&) {
            is_resizing_ = false;
        };
        on_close_ += [this](const auto&) {
            should_close_ = true;
        };
        on_maximize_ += [this](const auto&) {
            is_maximized_ = true;
            is_minimized_ = false;
        };
        on_minimize_ += [this](const auto&) {
            is_minimized_ = true;
            is_maximized_ = false;
        };
        on_restore_ += [this](const auto&) {
            is_maximized_ = false;
            is_minimized_ = false;
        };

        SPDLOG_LOGGER_DEBUG(logger(), "Window \"{}\" created", name_);
    }

    void Window::poll_events()
    {
        platform::update_window(platform_window_.get());
    }

    spdlog::logger* Window::logger()
    {
        static const auto window_logger = create_logger("orion-window", static_cast<spdlog::level::level_enum>(ORION_WINDOW_LOG_LEVEL));
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
