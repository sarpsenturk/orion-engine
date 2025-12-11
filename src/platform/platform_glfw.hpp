#pragma once

#include <GLFW/glfw3.h>

#include <array>

namespace orion
{
    struct Window {
        GLFWwindow* window;
        int width;
        int height;

        std::array<int, GLFW_KEY_LAST> key_states;
        std::array<int, GLFW_MOUSE_BUTTON_LAST> mouse_button_states;
        double cursor_xpos, cursor_ypos;
        double scroll_xoffset, scroll_yoffet;
        bool resized;
    };
} // namespace orion
