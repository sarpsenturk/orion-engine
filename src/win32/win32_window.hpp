#pragma once

#include "orion/window.hpp"

#include "orion_win32.hpp"

namespace orion
{
    struct PlatformWindow {
        HWND hwnd;
        WindowEvent event;
    };

    using Win32Window = PlatformWindow;
} // namespace orion
