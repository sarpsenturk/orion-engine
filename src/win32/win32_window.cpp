#include "win32_window.hpp"

#include "orion/assertion.hpp"

namespace orion
{
    namespace
    {
        constexpr const char* wnd_class_name = "OrionWindowClass";

        LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            if (auto* window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))) {
                switch (msg) {
                    case WM_CLOSE:
                        window->event = OnWindowClose{};
                        return 0;
                    case WM_SIZE:
                        window->event = OnWindowResize{.width = LOWORD(lparam), .height = HIWORD(lparam)};
                        return 0;
                    case WM_MOVE:
                        window->event = OnWindowMove{.xpos = static_cast<std::int32_t>(LOWORD(lparam)), .ypos = static_cast<std::int32_t>(HIWORD(lparam))};
                        return 0;
                    default:
                        break;
                }
            }
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }

        std::unique_ptr<PlatformWindow> create_win32_window(const WindowDesc& desc)
        {
            ORION_EXPECTS(desc.title != nullptr);
            ORION_EXPECTS(desc.width != UINT32_MAX);
            ORION_EXPECTS(desc.height != UINT32_MAX);

            // Register window class
            static const auto wnd_class_registered = []() {
                const auto wnd_class = WNDCLASS{
                    .lpfnWndProc = wnd_proc,
                    .hInstance = GetModuleHandle(nullptr),
                    .lpszClassName = wnd_class_name,
                };
                if (RegisterClass(&wnd_class) == 0) {
                    return GetLastError();
                } else {
                    return static_cast<DWORD>(ERROR_SUCCESS);
                }
            }();
            if (wnd_class_registered != ERROR_SUCCESS) {
                throw WindowCreateError{win32::format_last_error()};
            }

            // Set window style
            const auto window_style = WS_OVERLAPPEDWINDOW;

            // Adjust client size area to match requested dimensions
            auto rect = RECT{.left = 0, .top = 0, .right = static_cast<LONG>(desc.width), .bottom = static_cast<LONG>(desc.height)};
            if (!AdjustWindowRect(&rect, window_style, FALSE)) {
                throw WindowCreateError{win32::format_last_error()};
            }

            // Create PlatformWindow to pass to CreateWindow
            auto window = std::make_unique<Win32Window>();

            // Create the window
            HWND hwnd = CreateWindowEx(
                0,                                                                // Extended window style: https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
                wnd_class_name,                                                   // WNDCLASS class name
                desc.title,                                                       // Window name
                window_style,                                                     // Window style: https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
                desc.xpos != window_position_default ? desc.xpos : CW_USEDEFAULT, // X position
                desc.ypos != window_position_default ? desc.ypos : CW_USEDEFAULT, // Y position
                rect.right - rect.left,                                           // Width,
                rect.bottom - rect.top,                                           // Height
                NULL,                                                             // Parent window handle
                NULL,                                                             // Menu handle
                GetModuleHandle(nullptr),                                         // Module handle
                nullptr                                                           // User data pointer
            );
            if (hwnd == NULL) {
                throw WindowCreateError{win32::format_last_error()};
            }
            ShowWindow(hwnd, SW_SHOWNORMAL);
            window->hwnd = hwnd;

            // Set window user data pointer
            // Ugly reinterpret_cast but it works
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window.get()));

            return window;
        }

    } // namespace

    Window::Window(const WindowDesc& desc)
        : platform_window_(create_win32_window(desc))
    {
    }

    Window::Window(Window&&) noexcept = default;

    Window& Window::operator=(Window&&) noexcept = default;

    Window::~Window()
    {
        if (platform_window_) {
            DestroyWindow(platform_window_->hwnd);
        }
    }

    WindowEvent Window::poll_event()
    {
        // Empty last event
        platform_window_->event = std::monostate{};

        MSG msg;
        if (PeekMessage(&msg, platform_window_->hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return platform_window_->event;
    }
} // namespace orion
