#pragma once

#include "orion-core/event.h"
#include "orion-core/input.h"

#include <memory>
#include <string>

#include <orion-math/vector/vector2.h>

#include <spdlog/logger.h>

#ifndef ORION_WINDOW_LOG_LEVEL
    #define ORION_WINDOW_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

namespace orion
{
    // Window data type aliases
    using WindowPosition = Vector2_i;
    using WindowSize = Vector2_u;

    // Window create info
    struct WindowCreateInfo {
        std::string name = "Orion Window";
        WindowPosition position = {200, 200};
        WindowSize size = {800, 600};
    };

    // Forward declarations
    class Window;
    class PlatformWindow;

    // Platform specific functions. Implemented per platform
    namespace platform
    {
        // TODO: Return gsl::owner<PlatformWindow>
        PlatformWindow* create_window(Window* this_ptr, const WindowCreateInfo& window_create_info);
        void destroy_window(PlatformWindow* platform_window);
        void update_window(PlatformWindow* platform_window);
    } // namespace platform

    // unique_ptr to platform specific implementation
    using PlatformWindowPtr = std::unique_ptr<PlatformWindow, decltype(&platform::destroy_window)>;

    // Window events
    namespace events
    {
        struct WindowClose {
        };

        struct WindowMove {
            WindowPosition position;
        };

        struct WindowMoveEnd {
            WindowPosition position;
        };

        struct WindowResize {
            WindowSize size;
        };

        struct WindowResizeEnd {
            WindowSize size;
        };

        struct WindowMaximize {
        };

        struct WindowMinimize {
        };

        struct WindowRestore {
        };

        const char* format_as(const WindowClose&);
        std::string format_as(const WindowMove& move);
        std::string format_as(const WindowMoveEnd& move_end);
        std::string format_as(const WindowResize& resize);
        std::string format_as(const WindowResizeEnd& resize_end);
        const char* format_as(const WindowMaximize&);
        const char* format_as(const WindowMinimize&);
        const char* format_as(const WindowRestore&);
    } // namespace events

    class Window
    {
    public:
        explicit Window(WindowCreateInfo window_create_info);

        void poll_events();

        [[nodiscard]] auto* platform_window() const noexcept { return platform_window_.get(); }
        [[nodiscard]] auto& name() const noexcept { return name_; }
        [[nodiscard]] auto& position() const noexcept { return position_; }
        [[nodiscard]] auto& size() const noexcept { return size_; }
        [[nodiscard]] auto aspect_ratio() const noexcept { return static_cast<float>(size_.x()) / size_.y(); }
        [[nodiscard]] auto should_close() const noexcept { return should_close_; }

        [[nodiscard]] bool is_resizing() const noexcept { return is_resizing_; }
        [[nodiscard]] bool is_moving() const noexcept { return is_moving_; }
        [[nodiscard]] bool is_maximized() const noexcept { return is_maximized_; }
        [[nodiscard]] bool is_minimized() const noexcept { return is_minimized_; }

        [[nodiscard]] auto& on_close() noexcept { return on_close_; }
        [[nodiscard]] auto& on_move() noexcept { return on_move_; }
        [[nodiscard]] auto& on_move_end() noexcept { return on_move_end_; }
        [[nodiscard]] auto& on_resize() noexcept { return on_resize_; }
        [[nodiscard]] auto& on_resize_end() noexcept { return on_resize_end_; }
        [[nodiscard]] auto& on_maximize() noexcept { return on_maximize_; }
        [[nodiscard]] auto& on_minimize() noexcept { return on_minimize_; }
        [[nodiscard]] auto& on_restore() noexcept { return on_restore_; }

        [[nodiscard]] auto& keyboard() noexcept { return keyboard_; }
        [[nodiscard]] auto& keyboard() const noexcept { return keyboard_; }

    private:
        static spdlog::logger* logger();

        // Event dispatchers
        // These must be initialized before the platform window
        // because platform::create_window() may result in events being invoked
        // on certain platforms
        EventDispatcher<void(const events::WindowClose&)> on_close_;
        EventDispatcher<void(const events::WindowMove&)> on_move_;
        EventDispatcher<void(const events::WindowMoveEnd&)> on_move_end_;
        EventDispatcher<void(const events::WindowResize&)> on_resize_;
        EventDispatcher<void(const events::WindowResizeEnd&)> on_resize_end_;
        EventDispatcher<void(const events::WindowMaximize&)> on_maximize_;
        EventDispatcher<void(const events::WindowMinimize&)> on_minimize_;
        EventDispatcher<void(const events::WindowRestore&)> on_restore_;

        PlatformWindowPtr platform_window_;

        // Window information
        std::string name_;
        WindowPosition position_;
        WindowSize size_;
        bool should_close_ = false;

        // Window state information
        bool is_resizing_ = false;
        bool is_moving_ = false;
        bool is_maximized_ = false;
        bool is_minimized_ = false;

        // Window related input devices
        Keyboard keyboard_;
    };
} // namespace orion
