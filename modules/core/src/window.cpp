#include "orion-core/window.h"

#include <spdlog/sinks/stdout_color_sinks.h> // spdlog::stdout_color_st
#include <spdlog/spdlog.h>                   // SPDLOG_*

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
        on_move_ += [this](const events::WindowMove& move) {
            position_ = move.new_position;
            moving_ = true;
        };
        on_move_end_ += [this](const events::WindowMoveEnd&) {
            moving_ = false;
        };
        on_resize_ += [this](const events::WindowResize& resize) {
            size_ = resize.new_size;
            resizing_ = true;
        };
        on_resize_end_ += [this](const events::WindowResizeEnd&) {
            resizing_ = false;
        };
        on_close_ += [this](const events::WindowClose&) {
            should_close_ = true;
        };

        SPDLOG_LOGGER_DEBUG(logger(), "Window \"{}\" created", name_);
    }

    void Window::poll_events()
    {
        platform::update_window(platform_window_.get());
    }

    spdlog::logger* Window::logger()
    {
        const auto window_logger = []() {
            auto logger = spdlog::stdout_color_st("orion-window");
            logger->set_pattern("[%n] [%^%l%$] %v");
            logger->set_level(static_cast<spdlog::level::level_enum>(ORION_WINDOW_LOG_LEVEL));
            return logger;
        }();
        return window_logger.get();
    }
} // namespace orion
