#pragma once

#include "orion-core/event.h"
#include "orion-core/input.h"

#include "orion-utils/bits.h"

#include <memory>
#include <string>

#include <orion-math/vector/vector2.h>

#include <spdlog/logger.h>

namespace orion
{
    // Window data type aliases
    using WindowPosition = Vector2_i;
    using WindowSize = Vector2_u;

    // Window create info
    struct WindowCreateDesc {
        std::string name = "Orion Window";
        WindowPosition position;
        WindowSize size;
    };

    // Forward declarations
    class Window;
    class PlatformWindow;

    // Platform specific functions. Implemented per platform
    namespace platform
    {
        // TODO: Return gsl::owner<PlatformWindow>
        PlatformWindow* create_window(Window* this_ptr, const WindowCreateDesc& window_desc);
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
        explicit Window(WindowCreateDesc window_desc);

        void poll_events();

        [[nodiscard]] auto* platform_window() const noexcept { return platform_window_.get(); }
        [[nodiscard]] auto& name() const noexcept { return name_; }
        [[nodiscard]] auto& position() const noexcept { return position_; }
        [[nodiscard]] auto& size() const noexcept { return size_; }
        [[nodiscard]] auto aspect_ratio() const noexcept { return static_cast<float>(size_.x()) / size_.y(); }

        [[nodiscard]] bool should_close() const noexcept { return get_bit(state_, close_bit) != 0; }
        [[nodiscard]] bool is_resizing() const noexcept { return get_bit(state_, resize_bit) != 0; }
        [[nodiscard]] bool is_moving() const noexcept { return get_bit(state_, move_bit) != 0; }
        [[nodiscard]] bool is_maximized() const noexcept { return get_bit(state_, maximize_bit) != 0; }
        [[nodiscard]] bool is_minimized() const noexcept { return get_bit(state_, minimize_bit) != 0; }

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

        [[nodiscard]] auto& mouse() noexcept { return mouse_; }
        [[nodiscard]] auto& mouse() const noexcept { return mouse_; }

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

        // Window state information
        std::uint8_t state_;
        static constexpr auto close_bit = std::uint8_t{0};
        static constexpr auto resize_bit = std::uint8_t{1};
        static constexpr auto move_bit = std::uint8_t{2};
        static constexpr auto maximize_bit = std::uint8_t{3};
        static constexpr auto minimize_bit = std::uint8_t{4};

        // Window related input devices
        Keyboard keyboard_;
        Mouse mouse_;
    };
} // namespace orion
