#include "orion-core/platform/win32/win32_window.h"
#include "orion-core/platform/win32/win32_input.h"

#include "orion-utils/type.h"

#include <spdlog/spdlog.h>

#include <windowsx.h> // Needed for GET_X_LPARAM, GET_Y_LPARAM, etc.

namespace orion
{
    PlatformWindow::PlatformWindow(HWND hwnd, HINSTANCE hinstance)
        : hwnd_(hwnd)
        , hinstance_(hinstance)
    {
    }

    namespace platform
    {
        // Forward declare window procedure
        LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        namespace
        {
            void register_wnd_class(const char* class_name, HINSTANCE hinstance)
            {
                static bool registered = false;
                if (!registered) {
                    const WNDCLASS wndclass{
                        .lpfnWndProc = window_proc,
                        .hInstance = hinstance,
                        .lpszClassName = class_name,
                    };
                    if (!RegisterClass(&wndclass)) {
                        throw Win32Error();
                    }
                    SPDLOG_LOGGER_TRACE(win32::logger(), "Registered WNDCLASS {}", class_name);
                    registered = true;
                }
            }
        } // namespace

        PlatformWindow* create_window(Window* this_ptr, const WindowCreateInfo& window_create_info)
        {
            // Get the module instance
            HINSTANCE hinstance = GetModuleHandle(nullptr);

            // Register the window class
            const char* wnd_class_name = "Orion_WndClass";
            register_wnd_class(wnd_class_name, hinstance);

            // Set the window style TODO: Allow this to be customized?
            const auto window_style = WS_OVERLAPPEDWINDOW;

            // Adjust the window client area to match user specified size
            const auto size = [window_size = window_create_info.size]() {
                RECT wnd_rect{0, 0, static_cast<LONG>(window_size.x()), static_cast<LONG>(window_size.y())};
                AdjustWindowRect(&wnd_rect, window_style, FALSE);
                return Vector2_t<int>{(wnd_rect.right - wnd_rect.left), (wnd_rect.bottom - wnd_rect.top)};
            }();

            // Extract from window_create_info for easier reading
            const auto& position = window_create_info.position;

            // Create the window
            HWND hwnd = CreateWindowEx(
                0,                               // Optional window styles
                wnd_class_name,                  // Window class
                window_create_info.name.c_str(), // Window name
                window_style,                    // Window style
                position.x(),                    // Window position x
                position.y(),                    // Window position y
                size.x(),                        // Window width
                size.y(),                        // Window height
                nullptr,                         // Parent window
                nullptr,                         // Menu
                hinstance,                       // Instance handle
                this_ptr                         // Application data, ptr to Window
            );

            if (!hwnd) {
                throw Win32Error();
            }

            SPDLOG_LOGGER_TRACE(win32::logger(), "Created HWND {}", fmt::ptr(hwnd));

            // Show the window for the first time
            ShowWindow(hwnd, SW_SHOWNORMAL);

            // Return platform window
            return new PlatformWindow(hwnd, hinstance);
        }

        void destroy_window(PlatformWindow* platform_window)
        {
            if (platform_window) {
                HWND hwnd = platform_window->hwnd();
                DestroyWindow(hwnd);
                SPDLOG_LOGGER_TRACE(win32::logger(), "Destroyed HWND {}", fmt::ptr(hwnd));
            }
        }

        void update_window(PlatformWindow* platform_window)
        {
            if (platform_window) {
                MSG msg{};
                while (PeekMessage(&msg, platform_window->hwnd(), 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        // Window procedure implementation
        LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            // Set the window user ptr on create
            if (msg == WM_NCCREATE) {
                auto* create_struct = reinterpret_cast<CREATESTRUCT*>(lparam);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
                SPDLOG_LOGGER_TRACE(win32::logger(), "HWND ({}) GWLP_USERDATA set to this ptr", fmt::ptr(hwnd));
                return DefWindowProc(hwnd, msg, wparam, lparam);
            }

            // Retrieve window ptr on subsequent calls
            auto* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (!window) {
                return DefWindowProc(hwnd, msg, wparam, lparam);
            }

            // Helper lambdas
            static constexpr auto window_size = [](LPARAM lparam) -> WindowSize {
                return {LOWORD(lparam), HIWORD(lparam)};
            };
            static constexpr auto window_position = [](LPARAM lparam) -> WindowPosition {
                return {LOWORD(lparam), HIWORD(lparam)};
            };
            static constexpr auto mouse_position = [](LPARAM lparam) -> MousePosition {
                return {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
            };
            static constexpr auto wheel_delta = [](WPARAM wparam) -> int {
                return GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
            };
            static constexpr auto x_button = [](WPARAM wparam) -> MouseButton {
                const auto button = GET_XBUTTON_WPARAM(wparam);
                const auto offset = button - XBUTTON1;
                return static_cast<MouseButton>(to_underlying(MouseButton::X1) + offset);
            };

            // Windows message handler
            switch (msg) {
                case WM_CLOSE:
                    window->on_close().invoke({});
                    return 0;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
                case WM_SIZE:
                    window->on_resize().invoke({window_size(lparam)});
                    // WM_EXITSIZEMOVE is not sent after SIZE_MINIMIZED & SIZE_MAXIMIZED
                    switch (wparam) {
                        case SIZE_MINIMIZED:
                            window->on_resize_end().invoke({window->size()});
                            window->on_minimize().invoke({});
                            break;
                        case SIZE_MAXIMIZED:
                            window->on_resize_end().invoke({window->size()});
                            window->on_maximize().invoke({});
                            break;
                        case SIZE_RESTORED:
                            if (window->is_maximized() || window->is_minimized()) {
                                window->on_resize_end().invoke({window->size()});
                            }
                            window->on_restore().invoke({});
                            break;
                        default:
                            break;
                    }
                    return 0;
                case WM_MOVE:
                    window->on_move().invoke({window_position(lparam)});
                    return 0;
                case WM_EXITSIZEMOVE:
                    if (window->is_resizing()) {
                        window->on_resize_end().invoke({window->size()});
                    }
                    if (window->is_moving()) {
                        window->on_move_end().invoke({window->position()});
                    }
                    return 0;
                case WM_KEYDOWN:
                    // If 30th bit is set this is repeat message
                    if (lparam & (1ull << 30)) {
                        window->keyboard().on_key_repeat().invoke({.key = win32_vk_to_keycode(wparam)});
                    } else {
                        window->keyboard().on_key_press().invoke({.key = win32_vk_to_keycode(wparam)});
                    }
                    return 0;
                case WM_KEYUP:
                    window->keyboard().on_key_release().invoke({.key = win32_vk_to_keycode(wparam)});
                    return 0;
                case WM_LBUTTONDOWN:
                    window->mouse().on_button_down().invoke({.button = MouseButton::Left, .position = mouse_position(lparam)});
                    return 0;
                case WM_LBUTTONUP:
                    window->mouse().on_button_up().invoke({.button = MouseButton::Left, .position = mouse_position(lparam)});
                    return 0;
                case WM_MBUTTONDOWN:
                    window->mouse().on_button_down().invoke({.button = MouseButton::Middle, .position = mouse_position(lparam)});
                    return 0;
                case WM_MBUTTONUP:
                    window->mouse().on_button_up().invoke({.button = MouseButton::Middle, .position = mouse_position(lparam)});
                    return 0;
                case WM_RBUTTONDOWN:
                    window->mouse().on_button_down().invoke({.button = MouseButton::Right, .position = mouse_position(lparam)});
                    return 0;
                case WM_RBUTTONUP:
                    window->mouse().on_button_up().invoke({.button = MouseButton::Right, .position = mouse_position(lparam)});
                    return 0;
                case WM_MOUSEMOVE:
                    window->mouse().on_move().invoke({.position = mouse_position(lparam)});
                    return 0;
                case WM_MOUSEWHEEL:
                    // TODO: Convert screen coordinates to client coordinates
                    //  See: https://stackoverflow.com/questions/29915639/why-get-x-lparam-does-return-an-absolute-position-on-mouse-wheel
                    window->mouse().on_scroll().invoke({.delta = wheel_delta(wparam), .position = mouse_position(lparam)});
                    return 0;
                case WM_XBUTTONDOWN:
                    window->mouse().on_button_down().invoke({.button = x_button(wparam), .position = mouse_position(lparam)});
                    return 0;
                case WM_XBUTTONUP:
                    window->mouse().on_button_up().invoke({.button = x_button(wparam), .position = mouse_position(lparam)});
                    return 0;
                default:
                    break;
            }
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
    } // namespace platform
} // namespace orion
