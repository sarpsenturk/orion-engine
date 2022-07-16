#ifndef ORION_ENGINE_WIN_WINDOW_H
#define ORION_ENGINE_WIN_WINDOW_H

#include "orion/events/mouse_event.h"
#include "orion/events/window_event.h"
#include "orion/input/mouse.h"
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
        WinWindow(WinWindow&& other) noexcept;
        WinWindow& operator=(WinWindow&& other) noexcept;

        void initialize();
        void destroy();
        void update();
        [[nodiscard]] auto& props() const noexcept { return props_; }
        [[nodiscard]] bool should_close() const noexcept
        {
            return should_close_;
        }
        [[nodiscard]] bool is_valid() const noexcept
        {
            return hwnd_ != nullptr;
        }
        [[nodiscard]] auto& window_events() const noexcept
        {
            return window_events_;
        }
        [[nodiscard]] auto& window_events() noexcept { return window_events_; }
        [[nodiscard]] auto& mouse_events() const noexcept
        {
            return mouse_events_;
        }
        [[nodiscard]] auto& mouse_events() noexcept { return mouse_events_; }
        [[nodiscard]] auto& mouse() const noexcept { return mouse_; }

    private:
        static LRESULT CALLBACK main_window_procedure(HWND hwnd, UINT msg,
                                                      WPARAM wparam,
                                                      LPARAM lparam);
        static void register_win32_class(const char* class_name);
        [[nodiscard]] virtual const char* win32_class_name() const noexcept
        {
            return "orionWin32BaseWindow";
        }
        virtual LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam,
                                             LPARAM lparam);
        RECT get_window_rect() const;
        void invalidate_name();
        void invalidate_position(const RECT& rect);
        void invalidate_size(const RECT& rect);
        void invalidate_props();

    private:
        HWND hwnd_ = nullptr;
        bool should_close_ = false;
        WindowProps props_;
        WindowEventDispatcher window_events_;
        MouseEventDispatcher mouse_events_;
        Mouse mouse_;
    };
} // namespace orion::detail

#endif // ORION_ENGINE_WIN_WINDOW_H
