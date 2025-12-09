#include "orion/platform/window.hpp"

#include "orion/assert.hpp"
#include "orion/log.hpp"

#include <GLFW/glfw3.h>

namespace orion
{
    struct Window {
        GLFWwindow* window;
        int width;
        int height;
    };

    static void glfw_error_callback(int code, const char* description)
    {
        ORION_CORE_LOG_ERROR("GLFW error ({}): {}", code, description);
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
        glfwPollEvents();
    }

    bool platform_window_should_close(Window* window)
    {
        ORION_ASSERT(window != nullptr, "platform_window_should_close() expected window != nullptr");
        return glfwWindowShouldClose(window->window);
    }
} // namespace orion
