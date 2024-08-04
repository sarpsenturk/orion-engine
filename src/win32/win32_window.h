#pragma once

#include "orion/window.h"

#include "orion_win32.h"

namespace orion
{
    struct PlatformWindow {
        HWND hwnd;
        WindowEvent event;
    };

    using Win32Window = PlatformWindow;
} // namespace orion
