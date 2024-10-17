#include "win32_window.hpp"

#include "orion/assertion.hpp"

#include <Windowsx.h>

namespace orion
{
    namespace
    {
        constexpr const char* wnd_class_name = "OrionWindowClass";

        constexpr Keycode vk_to_keycode(WPARAM vk) noexcept
        {
            if (vk >= '0' && vk <= '9') {
                return static_cast<Keycode>(static_cast<int>(Keycode::Alpha0) + (vk - '0'));
            }
            if (vk >= 'A' && vk <= 'Z') {
                return static_cast<Keycode>(static_cast<int>(Keycode::KeyA) + (vk - 'A'));
            }
            switch (vk) {
                case VK_BACK:
                    return Keycode::Backspace;
                case VK_TAB:
                    return Keycode::Tab;
                case VK_RETURN:
                    return Keycode::Enter;
                case VK_ESCAPE:
                    return Keycode::Escape;
                case VK_SPACE:
                    return Keycode::Space;
                case VK_OEM_7:
                    return Keycode::Quote;
                case VK_OEM_COMMA:
                    return Keycode::Comma;
                case VK_OEM_MINUS:
                    return Keycode::Minus;
                case VK_OEM_PERIOD:
                    return Keycode::Period;
                case VK_OEM_2:
                    return Keycode::Slash;
                case VK_OEM_1:
                    return Keycode::Semicolon;
                case VK_OEM_PLUS:
                    return Keycode::Equal;
                case VK_OEM_4:
                    return Keycode::LeftBracket;
                case VK_OEM_5:
                    return Keycode::Backslash;
                case VK_OEM_6:
                    return Keycode::RightBracket;
                case VK_OEM_3:
                    return Keycode::Backtick;
                case VK_DELETE:
                    return Keycode::Delete;
                case VK_INSERT:
                    return Keycode::Insert;
                case VK_HOME:
                    return Keycode::Home;
                case VK_END:
                    return Keycode::End;
                case VK_PRIOR:
                    return Keycode::PageUp;
                case VK_NEXT:
                    return Keycode::PageDown;
                case VK_SNAPSHOT:
                    return Keycode::PrintScreen;
                case VK_SCROLL:
                    return Keycode::ScrollLock;
                case VK_PAUSE:
                    return Keycode::Pause;
                case VK_F1:
                    return Keycode::F1;
                case VK_F2:
                    return Keycode::F2;
                case VK_F3:
                    return Keycode::F3;
                case VK_F4:
                    return Keycode::F4;
                case VK_F5:
                    return Keycode::F5;
                case VK_F6:
                    return Keycode::F6;
                case VK_F7:
                    return Keycode::F7;
                case VK_F8:
                    return Keycode::F8;
                case VK_F9:
                    return Keycode::F9;
                case VK_F10:
                    return Keycode::F10;
                case VK_F11:
                    return Keycode::F11;
                case VK_F12:
                    return Keycode::F12;
                case VK_CAPITAL:
                    return Keycode::Caps;
                case VK_SHIFT:
                    return Keycode::Shift;
                case VK_CONTROL:
                    return Keycode::Control;
                case VK_MENU:
                    return Keycode::Alt;
                case VK_LWIN:
                    return Keycode::Command; // Usually maps to the left Windows key
                case VK_NUMPAD0:
                    return Keycode::Num0;
                case VK_NUMPAD1:
                    return Keycode::Num1;
                case VK_NUMPAD2:
                    return Keycode::Num2;
                case VK_NUMPAD3:
                    return Keycode::Num3;
                case VK_NUMPAD4:
                    return Keycode::Num4;
                case VK_NUMPAD5:
                    return Keycode::Num5;
                case VK_NUMPAD6:
                    return Keycode::Num6;
                case VK_NUMPAD7:
                    return Keycode::Num7;
                case VK_NUMPAD8:
                    return Keycode::Num8;
                case VK_NUMPAD9:
                    return Keycode::Num9;
                case VK_DIVIDE:
                    return Keycode::NumDivide;
                case VK_MULTIPLY:
                    return Keycode::NumMultiply;
                case VK_SUBTRACT:
                    return Keycode::NumMinus;
                case VK_ADD:
                    return Keycode::NumPlus;
                case VK_DECIMAL:
                    return Keycode::NumPeriod;
                case VK_NUMLOCK:
                    return Keycode::NumLock;
                case VK_LEFT:
                    return Keycode::LeftArrow;
                case VK_UP:
                    return Keycode::UpArrow;
                case VK_RIGHT:
                    return Keycode::RightArrow;
                case VK_DOWN:
                    return Keycode::DownArrow;
                default:
                    return Keycode::Unknown; // For undefined keys
            }
        }

        constexpr MouseButton get_mouse_button(UINT msg)
        {
            switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_LBUTTONDBLCLK:
                    return MouseButton::Left;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_RBUTTONDBLCLK:
                    return MouseButton::Right;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MBUTTONDBLCLK:
                    return MouseButton::Middle;
            }
            unreachable();
        }

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
                    case WM_KEYDOWN:
                        window->event = OnKeyDown{.keycode = vk_to_keycode(wparam)};
                        return 0;
                    case WM_KEYUP:
                        window->event = OnKeyUp{.keycode = vk_to_keycode(wparam)};
                        return 0;
                    case WM_MOUSEMOVE:
                        window->event = OnMouseMove{.xpos = GET_X_LPARAM(lparam), .ypos = GET_Y_LPARAM(lparam)};
                        return 0;
                    case WM_LBUTTONDOWN:
                    case WM_RBUTTONDOWN:
                    case WM_MBUTTONDOWN:
                        window->event = OnMouseButtonDown{.button = get_mouse_button(msg), .xpos = GET_X_LPARAM(lparam), .ypos = GET_Y_LPARAM(lparam)};
                        return 0;
                    case WM_LBUTTONUP:
                    case WM_RBUTTONUP:
                    case WM_MBUTTONUP:
                        window->event = OnMouseButtonUp{.button = get_mouse_button(msg), .xpos = GET_X_LPARAM(lparam), .ypos = GET_Y_LPARAM(lparam)};
                        return 0;
                    case WM_MOUSEWHEEL:
                        window->event = OnMouseScroll{.scroll = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA, .xpos = GET_X_LPARAM(lparam), .ypos = GET_Y_LPARAM(lparam)};
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

    WindowEvent Window::poll_event_impl()
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
