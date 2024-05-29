#include "orion-win32/win32_window.h"
#include "orion-win32/win32_input.h"

#include "orion-platform/platform.h"

#include "orion-utils/assertion.h"
#include "orion-utils/type.h"

#include <spdlog/spdlog.h>

#include <windowsx.h> // Needed for GET_X_LPARAM, GET_Y_LPARAM, etc.

namespace orion
{
    void PlatformWindow::exitsizemove()
    {
        resize_event = std::monostate{};
        move_event = std::monostate{};
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
                        throw_last_error();
                    }
                    SPDLOG_LOGGER_TRACE(platform_logger(), "Registered WNDCLASS {}", class_name);
                    registered = true;
                }
            }

            std::uint8_t resize_type(WPARAM wparam) noexcept
            {
                switch (wparam) {
                    case SIZE_RESTORED:
                        return 0;
                    case SIZE_MINIMIZED:
                        return window_minimized;
                    case SIZE_MAXIMIZED:
                        return window_maximized;
                }
                unreachable();
            }

            MousePosition mouse_position(LPARAM lparam) noexcept
            {
                return {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
            }

            MouseButton get_x_button(WPARAM wparam) noexcept
            {
                const auto button = GET_XBUTTON_WPARAM(wparam);
                const auto offset = button - XBUTTON1;
                return static_cast<MouseButton>(to_underlying(MouseButton::X1) + offset);
            }

            MouseButton get_mouse_button(UINT msg, WPARAM wparam) noexcept
            {
                switch (msg) {
                    case WM_LBUTTONDOWN:
                    case WM_LBUTTONUP:
                        return MouseButton::Left;
                    case WM_MBUTTONDOWN:
                    case WM_MBUTTONUP:
                        return MouseButton::Middle;
                    case WM_RBUTTONDOWN:
                    case WM_RBUTTONUP:
                        return MouseButton::Right;
                    case WM_XBUTTONDOWN:
                    case WM_XBUTTONUP:
                        return get_x_button(wparam);
                }
                unreachable();
            }

            int wheel_delta(WPARAM wparam) noexcept
            {
                return GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
            }
        } // namespace

        PlatformWindow* create_window(const WindowDesc& window_desc)
        {
            // Get the module instance
            HINSTANCE hinstance = GetModuleHandle(nullptr);

            // Register the window class
            const char* wnd_class_name = "Orion_WndClass";
            register_wnd_class(wnd_class_name, hinstance);

            // Set the window style TODO: Allow this to be customized?
            const auto window_style = WS_OVERLAPPEDWINDOW;

            // Adjust the window client area to match user specified size
            const auto size = [window_size = window_desc.size]() {
                RECT wnd_rect{0, 0, static_cast<LONG>(window_size.x()), static_cast<LONG>(window_size.y())};
                AdjustWindowRect(&wnd_rect, window_style, FALSE);
                return Vector2_t<int>{(wnd_rect.right - wnd_rect.left), (wnd_rect.bottom - wnd_rect.top)};
            }();

            // Extract from window_create_info for easier reading
            const auto& position = window_desc.position;

            // Create the window
            auto* platform_window = new PlatformWindow{.hinstance = hinstance};
            HWND hwnd = CreateWindowEx(
                0,                        // Optional window styles
                wnd_class_name,           // Window class
                window_desc.name.c_str(), // Window name
                window_style,             // Window style
                position.x(),             // Window position x
                position.y(),             // Window position y
                size.x(),                 // Window width
                size.y(),                 // Window height
                nullptr,                  // Parent window
                nullptr,                  // Menu
                hinstance,                // Instance handle
                platform_window           // Application data, ptr to Window
            );

            if (!hwnd) {
                throw_last_error();
            }
            platform_window->hwnd = hwnd;

            SPDLOG_LOGGER_TRACE(platform_logger(), "Created HWND {}", fmt::ptr(hwnd));

            // Show the window for the first time
            ShowWindow(hwnd, SW_SHOWNORMAL);

            // Return platform window
            return platform_window;
        }

        void destroy_window(PlatformWindow* platform_window)
        {
            if (platform_window) {
                HWND hwnd = platform_window->hwnd;
                DestroyWindow(hwnd);
                delete platform_window;
                SPDLOG_LOGGER_TRACE(platform_logger(), "Destroyed HWND {}", fmt::ptr(hwnd));
            }
        }

        WindowEvent poll_event_window(PlatformWindow* platform_window)
        {
            MSG msg{};
            if (PeekMessage(&msg, platform_window->hwnd, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (!platform_window->events.empty()) {
                auto event = std::move(platform_window->events.front());
                platform_window->events.pop();
                return event;
            }
            return {};
        }

        // Window procedure implementation
        LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            // Set the window user ptr on create
            if (msg == WM_NCCREATE) {
                auto* create_struct = reinterpret_cast<CREATESTRUCT*>(lparam);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create_struct->lpCreateParams));
                SPDLOG_LOGGER_TRACE(platform_logger(), "HWND ({}) GWLP_USERDATA set to PlatformWindow*", fmt::ptr(hwnd));
                return DefWindowProc(hwnd, msg, wparam, lparam);
            }

            // Retrieve window ptr on subsequent calls
            auto* window = reinterpret_cast<PlatformWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (!window) {
                return DefWindowProc(hwnd, msg, wparam, lparam);
            }

            switch (msg) {
                case WM_CLOSE:
                    window->events.push({WindowClose{}});
                    return 0;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
                case WM_SIZE:
                    window->resize_event = WindowResize{.size = {LOWORD(lparam), HIWORD(lparam)}};
                    return 0;
                case WM_MOVE:
                    window->move_event = WindowMove{.position = {LOWORD(lparam), HIWORD(lparam)}};
                    return 0;
                case WM_EXITSIZEMOVE:
                    if (window->resize_event) {
                        window->events.push(std::move(window->resize_event));
                    }
                    if (window->move_event) {
                        window->events.push(std::move(window->move_event));
                    }
                    window->exitsizemove();
                    return 0;
                case WM_ACTIVATE:
                    if (wparam == WA_ACTIVE || wparam == WA_CLICKACTIVE) {
                        window->events.push({WindowActivate{}});
                    } else if (wparam == WA_INACTIVE) {
                        window->events.push({WindowDeactivate{}});
                    }
                    return 0;
                case WM_KEYDOWN:
                    // 30th bit is set if key was down previously
                    if (lparam & (1ull << 30)) {
                        window->events.push({KeyRepeat{.key = win32_vk_to_keycode(wparam)}});
                    } else {
                        window->events.push({KeyDown{.key = win32_vk_to_keycode(wparam)}});
                    }
                    return 0;
                case WM_KEYUP:
                    window->events.push({KeyUp{.key = win32_vk_to_keycode(wparam)}});
                    return 0;
                case WM_LBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_XBUTTONDOWN:
                    window->events.push({MouseButtonDown{.button = get_mouse_button(msg, wparam), .position = mouse_position(lparam)}});
                    return 0;
                case WM_LBUTTONUP:
                case WM_MBUTTONUP:
                case WM_RBUTTONUP:
                case WM_XBUTTONUP:
                    window->events.push({MouseButtonUp{.button = get_mouse_button(msg, wparam), .position = mouse_position(lparam)}});
                    return 0;
                case WM_MOUSEMOVE:
                    window->events.push({MouseMove{.position = mouse_position(lparam)}});
                    return 0;
                case WM_MOUSEWHEEL:
                    window->events.push({MouseScroll{.delta = wheel_delta(wparam), .position = mouse_position(lparam)}});
                    return 0;
                default:
                    break;
            }
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
    } // namespace platform
} // namespace orion
