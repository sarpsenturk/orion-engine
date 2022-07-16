#include "orion/platform/win32/win_window.h"

#include <spdlog/spdlog.h>
#include <stdexcept>

namespace orion::detail
{
    WinWindow::WinWindow(WindowProps props)
        : props_(std::move(props))
    {
    }

    WinWindow::~WinWindow() { destroy(); }

    WinWindow::WinWindow(WinWindow&& other) noexcept
        : hwnd_(other.hwnd_)
        , should_close_(other.should_close_)
        , props_(std::move(other.props_))
    {
        other.hwnd_ = nullptr;
    }

    WinWindow& WinWindow::operator=(WinWindow&& other) noexcept
    {
        hwnd_ = other.hwnd_;
        should_close_ = other.should_close_;
        props_ = std::move(other.props_);

        other.hwnd_ = nullptr;
        return *this;
    }

    LRESULT WinWindow::main_window_procedure(HWND hwnd, UINT msg, WPARAM wparam,
                                             LPARAM lparam)
    {
        WinWindow* this_ptr = nullptr;
        if (msg == WM_NCCREATE) {
            auto create_params = reinterpret_cast<CREATESTRUCT*>(lparam);
            this_ptr = static_cast<WinWindow*>(create_params->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA,
                             reinterpret_cast<LONG>(this_ptr));
        } else {
            this_ptr = reinterpret_cast<WinWindow*>(
                GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (this_ptr) {
            return this_ptr->window_proc(hwnd, msg, wparam, lparam);
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    void WinWindow::initialize()
    {
        register_win32_class(win32_class_name());
        bool is_size_default = props_.size == WindowProps::kDefaultSize;
        bool is_position_default =
            props_.position == WindowProps::kDefaultPosition;
        hwnd_ = CreateWindowEx(
            0, win32_class_name(), props_.name.c_str(), WS_OVERLAPPEDWINDOW,
            is_position_default ? CW_USEDEFAULT : props_.position.x(),
            is_position_default ? CW_USEDEFAULT : props_.position.y(),
            is_size_default ? CW_USEDEFAULT : props_.size.x(),
            is_size_default ? CW_USEDEFAULT : props_.size.y(), nullptr, nullptr,
            GetModuleHandle(nullptr), this);
        if (!hwnd_) {
            throw std::runtime_error("Failed to create window");
        }
        ShowWindow(hwnd_, SW_SHOW);
        invalidate_props();
    }

    void WinWindow::destroy()
    {
        if (hwnd_) {
            DestroyWindow(hwnd_);
            hwnd_ = nullptr;
        }
    }

    void WinWindow::update()
    {
        if (!hwnd_) {
            return;
        }

        MSG msg{};
        while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void WinWindow::register_win32_class(const char* class_name)
    {
        WNDCLASS wc{};
        wc.lpfnWndProc = main_window_procedure;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = class_name;
        RegisterClass(&wc);
    }

    RECT WinWindow::get_window_rect() const
    {
        RECT window_rect{};
        GetWindowRect(hwnd_, &window_rect);
        return window_rect;
    }

    void WinWindow::invalidate_name()
    {
        auto window_text_length = GetWindowTextLength(hwnd_) + 1;
        props_.name.reserve(window_text_length);
        GetWindowText(hwnd_, props_.name.data(), window_text_length);
    }

    void WinWindow::invalidate_position(const RECT& rect)
    {
        props_.position = {rect.left, rect.top};
    }

    void WinWindow::invalidate_size(const RECT& rect)
    {
        props_.size = {rect.right - rect.left, rect.bottom - rect.top};
    }

    void WinWindow::invalidate_props()
    {
        invalidate_name();
        auto window_rect = get_window_rect();
        invalidate_size(window_rect);
        invalidate_position(window_rect);
    }

    LRESULT WinWindow::window_proc(HWND hwnd, UINT msg, WPARAM wparam,
                                   LPARAM lparam)
    {
        switch (msg) {
            case WM_CREATE:
                notify(WindowCreateEvent{props_.name});
                break;
            case WM_CLOSE:
                should_close_ = true;
                notify(WindowCloseEvent{props_.name});
                break;
            case WM_EXITSIZEMOVE: {
                auto rect = get_window_rect();
                invalidate_size(rect);
                invalidate_position(rect);
                notify(WindowResizeEvent{props_.name, props_.size});
                notify(WindowMoveEvent{props_.name, props_.position});
                break;
            }
            case WM_ACTIVATE: {
                bool is_focused =
                    wparam == WA_ACTIVE || wparam == WA_CLICKACTIVE;
                notify(WindowFocusEvent{props_.name, is_focused});
                break;
            }
            default:
                break;
        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
} // namespace orion::detail
