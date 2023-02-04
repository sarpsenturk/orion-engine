#pragma once

#include "orion-core/window.h"
#include "win32_headers.h"

namespace orion
{
    class PlatformWindow
    {
    public:
        PlatformWindow(HWND hwnd, HINSTANCE hinstance);

        [[nodiscard]] auto hwnd() const noexcept { return hwnd_; }
        [[nodiscard]] auto hinstance() const noexcept { return hinstance_; }

        bool resizing = false;
        bool moving = false;

        WindowSize cached_size{};
        WindowPosition cached_position{};

    private:
        HWND hwnd_;
        HINSTANCE hinstance_;
    };
} // namespace orion
