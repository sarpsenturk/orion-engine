#include "orion/platform/win32/win_window.h"

#include <stdexcept>

namespace orion::detail
{
    WinWindow::WinWindow(WindowProps props)
        : props_(std::move(props))
    {
    }

    WinWindow::~WinWindow() { destroy(); }

    LRESULT WinWindow::main_window_procedure(HWND hwnd, UINT msg, WPARAM wparam,
                                             LPARAM lparam)
    {
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
            is_size_default ? CW_USEDEFAULT : props_.size.x(),
            is_size_default ? CW_USEDEFAULT : props_.size.y(),
            is_position_default ? CW_USEDEFAULT : props_.position.x(),
            is_position_default ? CW_USEDEFAULT : props_.position.y(), nullptr,
            nullptr, GetModuleHandle(nullptr), this);
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

    void WinWindow::invalidate_props()
    {
        auto window_text_length = GetWindowTextLength(hwnd_) + 1;
        props_.name.reserve(window_text_length);
        GetWindowText(hwnd_, props_.name.data(), window_text_length);

        RECT window_rect{};
        GetWindowRect(hwnd_, &window_rect);
        props_.size = {window_rect.right - window_rect.left,
                       window_rect.bottom - window_rect.top};
        props_.position = {window_rect.left, window_rect.top};
    }
} // namespace orion::detail
