#ifndef ORION_ENGINE_WIN_WINDOW_H
#define ORION_ENGINE_WIN_WINDOW_H

#include "orion/window/window_props.h"
#include "win_include.h"

namespace orion::detail
{
    class WinWindow
    {
    public:
        explicit WinWindow(WindowProps props);
        virtual ~WinWindow();
        WinWindow(const WinWindow&) = delete;
        WinWindow& operator=(const WinWindow&) = delete;
        WinWindow(WinWindow&&) = default;
        WinWindow& operator=(WinWindow&&) = default;

        void initialize();
        void destroy();
        void update();
        [[nodiscard]] auto& props() const noexcept { return props_; }
        [[nodiscard]] bool should_close() const noexcept
        {
            return should_close_;
        }
        [[nodiscard]] bool is_valid() const noexcept { return hwnd_ != nullptr; }

    private:
        static LRESULT CALLBACK main_window_procedure(HWND hwnd, UINT msg,
                                                      WPARAM wparam,
                                                      LPARAM lparam);
        static void register_win32_class(const char* class_name);
        [[nodiscard]] virtual const char* win32_class_name() const noexcept
        {
            return "orionWin32BaseWindow";
        }
        void invalidate_props();

    private:
        HWND hwnd_ = nullptr;
        bool should_close_ = false;
        WindowProps props_;
    };
} // namespace orion::detail

#endif // ORION_ENGINE_WIN_WINDOW_H
