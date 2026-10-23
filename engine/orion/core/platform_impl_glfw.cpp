#include "event.hpp"
#include "platform.hpp"
#include "window.hpp"

#include "orion/core/assert.hpp"

#include <GLFW/glfw3.h>

#include <queue>
#include <stdexcept>
#include <utility>

namespace orion
{
    static std::queue<Event> g_event_queue;

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

    void Platform::pump_events()
    {
        ORION_ASSERT(g_event_queue.empty() && "All previous events should be processed before pump_events() is called");
        glfwPollEvents();
    }

    bool Platform::poll_event(Event& event)
    {
        if (g_event_queue.empty()) {
            return false;
        } else {
            event = std::move(g_event_queue.front());
            g_event_queue.pop();
            return true;
        }
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
        glfwSetWindowCloseCallback(window, [](GLFWwindow* window_) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window_));
            g_event_queue.push(OnWindowClose{self});
        });
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window_, int width, int height) {
            auto* self     = static_cast<Window*>(glfwGetWindowUserPointer(window_));
            self->m_width  = width;
            self->m_height = height;

            // Coalesce only adjacent events to preserve ordering with other events.
            if (!g_event_queue.empty()) {
                auto* pending = std::get_if<OnWindowResize>(&g_event_queue.back());
                if (pending && pending->window == self) {
                    *pending = OnWindowResize{self, width, height};
                    return;
                }
            }
            g_event_queue.push(OnWindowResize{self, width, height});
        });
        glfwSetWindowPosCallback(window, [](GLFWwindow* window_, int x, int y) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window_));
            if (!g_event_queue.empty()) {
                auto* pending = std::get_if<OnWindowMove>(&g_event_queue.back());
                if (pending && pending->window == self) {
                    *pending = OnWindowMove{self, x, y};
                    return;
                }
            }
            g_event_queue.push(OnWindowMove{self, x, y});
        });
        glfwSetKeyCallback(window, [](GLFWwindow* window_, int key, int /*scancode*/, int action, int /*mods*/) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window_));
            switch (action) {
                case GLFW_RELEASE:
                    g_event_queue.push(OnKeyRelease{self, static_cast<Keycode>(key)});
                    break;
                case GLFW_PRESS:
                    g_event_queue.push(OnKeyPress{self, static_cast<Keycode>(key)});
                    break;
                case GLFW_REPEAT:
                    g_event_queue.push(OnKeyRepeat{self, static_cast<Keycode>(key)});
                    break;
                default:
                    std::unreachable();
            }
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* window_, int button, int action, int /*mods*/) {
            auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window_));
            switch (action) {
                case GLFW_RELEASE:
                    g_event_queue.push(OnMouseButtonRelease{self, static_cast<MouseButton>(button)});
                    break;
                case GLFW_PRESS:
                    g_event_queue.push(OnMouseButtonPress{self, static_cast<MouseButton>(button)});
                    break;
                default:
                    std::unreachable();
            }
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

    bool Window::is_key_pressed(Keycode key) const noexcept
    {
        return glfwGetKey(static_cast<GLFWwindow*>(m_handle), static_cast<int>(key)) == GLFW_PRESS;
    }

    bool Window::is_mouse_button_pressed(MouseButton button) const noexcept
    {
        return glfwGetMouseButton(static_cast<GLFWwindow*>(m_handle), static_cast<int>(button)) == GLFW_PRESS;
    }

    MousePosition Window::mouse_position() const noexcept
    {
        MousePosition pos;
        glfwGetCursorPos(static_cast<GLFWwindow*>(m_handle), &pos.x, &pos.y);
        return pos;
    }
} // namespace orion
