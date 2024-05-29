#pragma once

#include "orion-platform/window.h"
#include "orion-win32/win32_platform.h"

#include <queue>

namespace orion
{
    class PlatformWindow
    {
    public:
        HWND hwnd;
        HINSTANCE hinstance;
        std::queue<WindowEvent> events;

        WindowEvent resize_event = {};
        WindowEvent move_event = {};

        void exitsizemove();
    };
} // namespace orion
