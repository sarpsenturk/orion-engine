#include "orion-core/platform/win32/win32_window.h"

#include <spdlog/spdlog.h> // SPDLOG_*

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

        static void register_wnd_class(const char* class_name, HINSTANCE hinstance)
        {
            static bool registered = false;
            if (!registered) {
                const WNDCLASS wndclass{
                    .lpfnWndProc = window_proc,
                    .hInstance = hinstance,
                    .lpszClassName = class_name,
                };
                RegisterClass(&wndclass); // TODO: Check return
                registered = true;
            }
        }

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
            const auto size = [window_style, window_size = window_create_info.size]() {
                RECT wnd_rect{0, 0, static_cast<LONG>(window_size.x()), static_cast<LONG>(window_size.y())};
                AdjustWindowRect(&wnd_rect, window_style, FALSE);
                return math::Vector2_t<int>{(wnd_rect.right - wnd_rect.left), (wnd_rect.bottom - wnd_rect.top)};
            }();

            // Extract from window_create_info for easier reading
            const auto& position = window_create_info.position;

            // Create the window
            HWND hwnd = CreateWindowEx(
                0,                               // Optional window styles
                wnd_class_name,                  // Window class
                window_create_info.name.c_str(), // Window name
                window_style,                    // Window style
                position.x(), position.y(),      // Window position
                size.x(), size.y(),              // Window size
                nullptr,                         // Parent window
                nullptr,                         // Menu
                hinstance,                       // Instance handle
                this_ptr                         // Application data, ptr to Window
            );

            // TODO: Check if hwnd is nullptr

            SPDLOG_TRACE("Created HWND ({})", fmt::ptr(hwnd));

            // Show the window for the first time
            ShowWindow(hwnd, SW_SHOWNORMAL);

            // Return platform window
            return new PlatformWindow(hwnd, hinstance);
        }

        void destroy_window(PlatformWindow* platform_window)
        {
            if (platform_window) {
                DestroyWindow(platform_window->hwnd());
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
                return DefWindowProc(hwnd, msg, wparam, lparam);
            }

            [[maybe_unused]] auto* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            // TODO: Implement message handling with events
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
    } // namespace platform
} // namespace orion
