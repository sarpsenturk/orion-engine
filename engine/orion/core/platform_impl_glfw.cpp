#include "platform.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <utility>

namespace orion
{
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
} // namespace orion
