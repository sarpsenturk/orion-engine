#include "input.hpp"
#include "platform.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <utility>

namespace orion
{
    static int to_glfw_cursor_mode(CursorMode mode)
    {
        switch (mode) {
            case CursorMode::Normal:
                return GLFW_CURSOR_NORMAL;
            case CursorMode::Hidden:
                return GLFW_CURSOR_HIDDEN;
            case CursorMode::Disabled:
                return GLFW_CURSOR_DISABLED;
            case CursorMode::Captured:
                return GLFW_CURSOR_CAPTURED;
        }
        std::unreachable();
    }

    Platform::Platform()
    {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
    }

    Platform::~Platform()
    {
        glfwTerminate();
    }

    void Platform::poll_events()
    {
        glfwPollEvents();
    }

    const char* Platform::name() const noexcept
    {
        return "GLFW";
    }

    const char* Platform::version() const noexcept
    {
        return glfwGetVersionString();
    }

    Window::Window(const WindowDesc& desc)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable OpenGL context creation
        GLFWwindow* window = glfwCreateWindow(desc.width, desc.height, desc.title, nullptr, nullptr);
        if (window == nullptr) {
            throw std::runtime_error("Failed to create GLFW window");
        } else {
            m_handle = window;
        }

        // Use framebuffer size to get the actual size of the window in pixels
        glfwGetFramebufferSize(window, &m_width, &m_height);

        // Set user pointer and callbacks
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window_, int width, int height) {
            auto* self     = static_cast<Window*>(glfwGetWindowUserPointer(window_));
            self->m_width  = width;
            self->m_height = height;
        });
    }

    Window::Window(Window&& other) noexcept
        : m_handle(std::exchange(other.m_handle, nullptr))
        , m_width(other.m_width)
        , m_height(other.m_height)
    {
        glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_handle), this);
    }

    Window& Window::operator=(Window&& other) noexcept
    {
        if (this != &other) {
            if (m_handle) {
                glfwDestroyWindow(static_cast<GLFWwindow*>(m_handle));
            }
            m_handle = std::exchange(other.m_handle, nullptr);
            m_width  = other.m_width;
            m_height = other.m_height;
            glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_handle), this);
        }
        return *this;
    }

    Window::~Window()
    {
        if (m_handle) {
            glfwDestroyWindow(static_cast<GLFWwindow*>(m_handle));
        }
    }

    bool Window::should_close() const noexcept
    {
        return glfwWindowShouldClose(static_cast<GLFWwindow*>(m_handle));
    }

    void Window::set_cursor_mode(CursorMode mode)
    {
        auto* window = static_cast<GLFWwindow*>(m_handle);
        glfwSetInputMode(window, GLFW_CURSOR, to_glfw_cursor_mode(mode));
    }

    Input::Input(Window* window)
        : m_window(window)
    {
    }

    bool Input::is_key_pressed(Keycode key) const
    {
        auto* window = static_cast<GLFWwindow*>(m_window->handle());
        return glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS;
    }

    bool Input::is_mouse_button_pressed(MouseButton button) const
    {
        auto* window = static_cast<GLFWwindow*>(m_window->handle());
        return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS;
    }

    MousePosition Input::mouse_position() const
    {
        auto* window = static_cast<GLFWwindow*>(m_window->handle());
        MousePosition position;
        glfwGetCursorPos(window, &position.x, &position.y);
        return position;
    }
} // namespace orion
