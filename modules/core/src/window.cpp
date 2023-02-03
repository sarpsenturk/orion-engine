#include "orion-core/window.h"

namespace orion
{
    Window::Window(WindowCreateInfo window_create_info)
        : platform_window_(platform::create_window(this, window_create_info), platform::destroy_window)
        , name_(std::move(window_create_info.name))
        , position_(window_create_info.position)
        , size_(window_create_info.size)
    {
    }

    void Window::poll_events()
    {
        platform::update_window(platform_window_.get());
    }
} // namespace orion
