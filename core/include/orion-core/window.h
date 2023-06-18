#pragma once

#include "orion-core/event.h"
#include "orion-core/input.h"

#include <memory>                        // std::shared_ptr
#include <orion-math/vector/formatter.h> // fmt::formatter<Vector>
#include <orion-math/vector/vector2.h>   // orion::math::Vector2
#include <spdlog/logger.h>               // spdlog::logger

#ifndef ORION_WINDOW_LOG_LEVEL
    #define ORION_WINDOW_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

namespace orion
{
    // Window data type aliases
    using WindowPosition = math::Vector2_i;
    using WindowSize = math::Vector2_u;

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

// Event formatters for fmt
template<>
struct fmt::formatter<orion::events::WindowClose> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowClose&, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowClose");
    }
};

template<>
struct fmt::formatter<orion::events::WindowMove> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowMove& move, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowMove {{ position: {} }}", move.position);
    }
};

template<>
struct fmt::formatter<orion::events::WindowMoveEnd> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowMoveEnd& move, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowMoveEnd {{ final_position: {} }}", move.position);
    }
};

template<>
struct fmt::formatter<orion::events::WindowResize> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowResize& resize, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowResize {{ size: {} }}", resize.size);
    }
};

template<>
struct fmt::formatter<orion::events::WindowResizeEnd> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowResizeEnd& resize, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowResizeEnd {{ final_size: {} }}", resize.size);
    }
};

template<>
struct fmt::formatter<orion::events::WindowMaximize> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowMaximize&, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowMaximize");
    }
};

template<>
struct fmt::formatter<orion::events::WindowMinimize> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowMinimize&, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowMinimize");
    }
};

template<>
struct fmt::formatter<orion::events::WindowRestore> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowRestore&, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowRestore");
    }
};
