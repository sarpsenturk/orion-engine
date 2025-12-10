#pragma once

namespace orion
{
    // Opaque window structure
    struct Window;

    Window* platform_window_create(const char* title, int width, int height);
    void platform_window_destroy(Window* window);
    void platform_window_poll_events(Window* window);
    bool platform_window_should_close(const Window* window);
    bool platform_window_was_resized(const Window* window);
    void platform_window_clear_resized(Window* window);
    void platform_window_get_size(const Window* window, int* width, int* height);
} // namespace orion
