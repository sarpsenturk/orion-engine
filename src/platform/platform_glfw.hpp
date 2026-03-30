#pragma once

#include "orion/event.hpp"
#include "orion/window.hpp"

#include <GLFW/glfw3.h>

namespace orion
{
    struct Window::Impl {
        GLFWwindow* window;
        int width;
        int height;
        Event<OnWindowClose> on_close;
        Event<OnWindowMove> on_move;
        Event<OnWindowResize> on_resize;

        Impl(GLFWwindow* _window, int _width, int _height);
        Impl(const Impl&) = delete;
        Impl& operator=(const Impl&) = delete;
        Impl(Impl&& other) noexcept;
        Impl& operator=(Impl&& other) noexcept;
        ~Impl();
    };
} // namespace orion
