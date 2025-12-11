#include "platform_glfw.hpp"

#include "orion/platform/input.hpp"
#include "orion/platform/platform.hpp"
#include "orion/platform/time.hpp"
#include "orion/platform/window.hpp"

#include "orion/assert.hpp"
#include "orion/log.hpp"

#include <array>

namespace orion
{
    static void glfw_error_callback(int code, const char* description)
    {
        ORION_CORE_LOG_ERROR("GLFW error ({}): {}", code, description);
    }

    static void glfw_key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
    {
        auto* impl = static_cast<Window*>(glfwGetWindowUserPointer(window));
        impl->key_states[key] = action;
    }

    static void glfw_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
    {
        auto* impl = static_cast<Window*>(glfwGetWindowUserPointer(window));
        impl->cursor_xpos = xpos;
        impl->cursor_ypos = ypos;
    }

    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/)
    {
        auto* impl = static_cast<Window*>(glfwGetWindowUserPointer(window));
        impl->mouse_button_states[button] = action;
    }

    static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto* impl = static_cast<Window*>(glfwGetWindowUserPointer(window));
        impl->scroll_xoffset = xoffset;
        impl->scroll_yoffet = yoffset;
    }

    static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        auto* impl = static_cast<Window*>(glfwGetWindowUserPointer(window));
        impl->width = width;
        impl->height = height;
        impl->resized = true;
    }

    bool platform_init()
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (glfwInit()) {
            const char* glfw_version = glfwGetVersionString();
            ORION_CORE_LOG_INFO("GLFW initialized: {}", glfw_version);
            return true;
        } else {
            return false;
        }
    }

    void platform_shutdown()
    {
        glfwTerminate();
        ORION_CORE_LOG_INFO("GLFW terminated");
    }

    Window* platform_window_create(const char* title, int width, int height)
    {
        ORION_ASSERT(width > 0, "Window width must be greater than 0");
        ORION_ASSERT(height > 0, "Window height must be greater than 0");

        // Allocate new window
        Window* window = new Window;

        // Create window, no OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window->window = glfwCreateWindow(width, height, title, nullptr, nullptr);

        // Set window user pointer
        glfwSetWindowUserPointer(window->window, window);

        // Setup event callbacks
        glfwSetKeyCallback(window->window, glfw_key_callback);
        glfwSetCursorPosCallback(window->window, glfw_cursor_pos_callback);
        glfwSetMouseButtonCallback(window->window, glfw_mouse_button_callback);
        glfwSetScrollCallback(window->window, glfw_scroll_callback);
        glfwSetFramebufferSizeCallback(window->window, glfw_framebuffer_size_callback);

        // Get created framebuffer dimensions
        glfwGetFramebufferSize(window->window, &window->width, &window->height);
        if (window->width == width && window->height == height) {
            ORION_CORE_LOG_INFO("GLFWwindow* ({}) created {{ title: {}, width: {}, height: {} }}",
                                (void*)window->window, title, window->width, window->height);
        } else {
            ORION_CORE_LOG_WARN("GLFWwindow* ({}) was created with different dimensions than requested ({},{}): {{ title: {}, width: {}, height: {} }}",
                                (void*)window->window, width, height, title, window->width, window->height);
        }

        return window;
    }

    void platform_window_destroy(Window* window)
    {
        ORION_ASSERT(window != nullptr, "platform_window_destroy() expected window != nullptr");
        glfwDestroyWindow(window->window);
        ORION_CORE_LOG_INFO("GLFWwindow* ({}) destroyed", (void*)window->window);
        delete window;
    }

    void platform_window_poll_events(Window* window)
    {
        ORION_ASSERT(window != nullptr, "platform_window_poll_events() expected window != nullptr");
        std::memset(&window->key_states, GLFW_RELEASE, sizeof(window->key_states));
        std::memset(&window->mouse_button_states, GLFW_RELEASE, sizeof(window->mouse_button_states));
        window->scroll_xoffset = 0.0;
        window->scroll_yoffet = 0.0;
        glfwPollEvents();
    }

    bool platform_window_should_close(const Window* window)
    {
        ORION_ASSERT(window != nullptr, "platform_window_should_close() expected window != nullptr");
        return glfwWindowShouldClose(window->window);
    }

    bool platform_window_was_resized(const Window* window)
    {
        return window->resized;
    }

    void platform_window_clear_resized(Window* window)
    {
        window->resized = false;
    }

    void platform_window_get_size(const Window* window, int* width, int* height)
    {
        *width = window->width;
        *height = window->height;
    }

    bool platform_input_key_pressed(const Window* window, KeyCode key)
    {
        return window->key_states[static_cast<int>(key)] == GLFW_PRESS ||
               window->key_states[static_cast<int>(key)] == GLFW_REPEAT;
    }

    bool platform_input_key_released(const Window* window, KeyCode key)
    {
        return window->key_states[static_cast<int>(key)] == GLFW_RELEASE;
    }

    void platform_input_cursor_position(const Window* window, double* xpos, double* ypos)
    {
        *xpos = window->cursor_xpos;
        *ypos = window->cursor_ypos;
    }

    bool platform_input_mouse_button_pressed(const Window* window, MouseButton button)
    {
        return window->mouse_button_states[static_cast<int>(button)] == GLFW_PRESS;
    }

    bool platform_input_mouse_button_released(const Window* window, MouseButton button)
    {
        return window->mouse_button_states[static_cast<int>(button)] == GLFW_RELEASE;
    }

    void platform_input_scroll_delta(const Window* window, double* xdelta, double* ydelta)
    {
        *xdelta = window->scroll_xoffset;
        *ydelta = window->scroll_yoffet;
    }

    double platform_time_get_time()
    {
        return glfwGetTime();
    }
} // namespace orion
