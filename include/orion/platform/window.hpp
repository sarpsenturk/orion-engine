#pragma once

namespace orion
{
    // Opaque window structure
    struct Window;

    Window* platform_window_create(const char* title, int width, int height);
    void platform_window_destroy(Window* window);
    void platform_window_poll_events(Window* window);
    bool platform_window_should_close(Window* window);
} // namespace orion
