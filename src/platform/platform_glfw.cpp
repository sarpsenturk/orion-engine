#include "orion/window.hpp"

#include "orion/log.hpp"

#include <GLFW/glfw3.h>

#include <functional>
#include <stdexcept>
#include <utility>

namespace orion
{
    struct Window::Impl {
        GLFWwindow* window;
        int width;
        int height;
        std::function<void(const WindowEvent&)> on_event;

        Impl(GLFWwindow* _window, int _width, int _height)
            : window(_window)
            , width(_width)
            , height(_height)
        {
        }

        Impl(const Impl&) = delete;
        Impl& operator=(const Impl&) = delete;

        Impl(Impl&& other) noexcept
            : window(std::exchange(other.window, nullptr))
            , width(other.width)
            , height(other.height)
        {
        }

        Impl& operator=(Impl&& other) noexcept
        {
            if (this != &other) {
                window = std::exchange(other.window, nullptr);
                width = other.width;
                height = other.height;
            }
            return *this;
        }

        ~Impl()
        {
            if (window != nullptr) {
                glfwDestroyWindow(window);
                ORION_CORE_LOG_INFO("Destroyed GLFWwindow* {}", (void*)window);
                glfwTerminate();
                ORION_CORE_LOG_INFO("GLFW terminated");
            }
        }
    };

    Window::Window(const WindowDesc& desc)
    {
        // Initialize GLFW
        glfwSetErrorCallback([](int code, const char* description) { ORION_CORE_LOG_ERROR("GLFW error ({}): {}", code, description); });
        if (!glfwInit()) {
            throw std::runtime_error("glfwInit() failed");
        }
        ORION_CORE_LOG_INFO("GLFW initialized {}", glfwGetVersionString());

        // Create GLFW window with no client API (no OpenGL context)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(desc.width, desc.height, desc.title, nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("glfwCreateWindow failed");
        }
        ORION_CORE_LOG_INFO("Created GLFWwindow* {}", (void*)window);

        // Get created windows framebuffer dimensions
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (width != desc.width || height != desc.height) {
            ORION_CORE_LOG_WARN("Created window dimensions ({},{}) was different than requested ({},{})",
                                width, height,
                                desc.width, desc.height);
        }

        // Initialize impl
        impl_ = std::make_unique<Impl>(window, width, height);

        // Set GLFWwindow user pointer to impl ptr for event callbacks
        glfwSetWindowUserPointer(window, impl_.get());

        // Set GLFW event handlers
        glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
            static_cast<Window::Impl*>(glfwGetWindowUserPointer(window))->on_event({OnWindowClose{}});
        });
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            static_cast<Window::Impl*>(glfwGetWindowUserPointer(window))->on_event({OnWindowResize{width, height}});
        });
        glfwSetWindowPosCallback(window, [](GLFWwindow* window, int xpos, int ypos) {
            static_cast<Window::Impl*>(glfwGetWindowUserPointer(window))->on_event({OnWindowMove{xpos, ypos}});
        });
    }

    Window::~Window() = default;

    int Window::width() const
    {
        return impl_->width;
    }

    int Window::height() const
    {
        return impl_->height;
    }

    bool Window::should_close() const
    {
        return glfwWindowShouldClose(impl_->window);
    }

    void Window::poll_events()
    {
        glfwPollEvents();
    }

    void Window::set_event_callback(std::function<void(const WindowEvent&)> callback)
    {
        impl_->on_event = std::move(callback);
    }
} // namespace orion
