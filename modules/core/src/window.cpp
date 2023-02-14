#include "orion-core/window.h"

#include <spdlog/spdlog.h> // SPDLOG_*

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

        SPDLOG_DEBUG("Window \"{}\" created", name_);
    }

    void Window::poll_events()
    {
        platform::update_window(platform_window_.get());
    }
} // namespace orion
