#include "platform_glfw.hpp"

#include "orion/log.hpp"

#include <GLFW/glfw3.h>

#include <utility>

namespace orion
{
    Window::Impl::Impl(GLFWwindow* _window, int _width, int _height)
        : window(_window)
        , width(_width)
        , height(_height)
    {
        ORION_LOG_EVENT(on_close);
        ORION_LOG_EVENT(on_move);
        ORION_LOG_EVENT(on_resize);
    }

    Window::Impl::Impl(Impl&& other) noexcept
        : window(std::exchange(other.window, nullptr))
        , width(other.width)
        , height(other.height)
    {
    }

    Window::Impl& Window::Impl::operator=(Impl&& other) noexcept
    {
        if (this != &other) {
            window = std::exchange(other.window, nullptr);
            width = other.width;
            height = other.height;
        }
        return *this;
    }

    Window::Impl::~Impl()
    {
        if (window != nullptr) {
            glfwDestroyWindow(window);
            ORION_CORE_LOG_INFO("Destroyed GLFWwindow* {}", (void*)window);
            glfwTerminate();
            ORION_CORE_LOG_INFO("GLFW terminated");
        }
    }

    tl::expected<Window, std::string> Window::initialize(const WindowDesc& desc)
    {
        // Initialize GLFW
        glfwSetErrorCallback([](int code, const char* description) { ORION_CORE_LOG_ERROR("GLFW error ({}): {}", code, description); });
        if (!glfwInit()) {
            return tl::unexpected("glfwInit() failed");
        }
        ORION_CORE_LOG_INFO("GLFW initialized {}", glfwGetVersionString());

        // Create GLFW window with no client API (no OpenGL context)
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(desc.width, desc.height, desc.title, nullptr, nullptr);
        if (!window) {
            return tl::unexpected("glfwCreateWindow failed");
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
        auto impl = std::make_unique<Impl>(window, width, height);

        // Set GLFWwindow user pointer to impl ptr for event callbacks
        glfwSetWindowUserPointer(window, impl.get());

        // Set GLFW event handlers
        glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
            static_cast<Window::Impl*>(glfwGetWindowUserPointer(window))->on_close.invoke(OnWindowClose{});
        });
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            auto* impl = static_cast<Window::Impl*>(glfwGetWindowUserPointer(window));
            impl->width = width;
            impl->height = height;
            impl->on_resize.invoke(OnWindowResize{width, height});
        });
        glfwSetWindowPosCallback(window, [](GLFWwindow* window, int xpos, int ypos) {
            static_cast<Window::Impl*>(glfwGetWindowUserPointer(window))->on_move.invoke(OnWindowMove{xpos, ypos});
        });

        return Window{std::move(impl)};
    }

    Window::Window(std::unique_ptr<Impl> impl)
        : impl_(std::move(impl))
    {
    }

    // Need to explicity default here where Window::Impl is defined
    Window::Window(Window&&) noexcept = default;
    Window& Window::operator=(Window&&) noexcept = default;

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

    Event<OnWindowClose>& Window::on_window_close()
    {
        return impl_->on_close;
    }

    Event<OnWindowMove>& Window::on_window_move()
    {
        return impl_->on_move;
    }

    Event<OnWindowResize>& Window::on_window_resize()
    {
        return impl_->on_resize;
    }
} // namespace orion
