#pragma once

#include "orion-platform/window.h"
#include "orion-win32/win32_platform.h"

namespace orion
{
    class PlatformWindow
    {
    public:
        HWND hwnd;
        HINSTANCE hinstance;
        WindowEvent event;
    };
} // namespace orion
