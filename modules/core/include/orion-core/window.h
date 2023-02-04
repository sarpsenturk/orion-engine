#pragma once

#include "orion-core/event.h"

#include <orion-math/vector/formatter.h>
#include <orion-math/vector/vector2.h>
#include <orion-utils/enum.h>

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
            WindowPosition new_position;
        };

        struct WindowMoveEnd {
            WindowPosition final_position;
        };

        struct WindowResize {
            WindowSize new_size;
        };

        struct WindowResizeEnd {
            WindowSize final_size;
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

        [[nodiscard]] bool resizing() const noexcept { return resizing_; }
        [[nodiscard]] bool moving() const noexcept { return moving_; }

        [[nodiscard]] auto& on_close() noexcept { return on_close_; }
        [[nodiscard]] auto& on_move() noexcept { return on_move_; }
        [[nodiscard]] auto& on_move_end() noexcept { return on_move_end_; }
        [[nodiscard]] auto& on_resize() noexcept { return on_resize_; }
        [[nodiscard]] auto& on_resize_end() noexcept { return on_resize_end_; }

    private:
        PlatformWindowPtr platform_window_;

        // Window information
        std::string name_;
        WindowPosition position_;
        WindowSize size_;
        bool should_close_ = false;

        // Window state information
        bool resizing_ = false;
        bool moving_ = false;

        // Event dispatchers
        EventDispatcher<void(const events::WindowClose&)> on_close_;
        EventDispatcher<void(const events::WindowMove&)> on_move_;
        EventDispatcher<void(const events::WindowMoveEnd&)> on_move_end_;
        EventDispatcher<void(const events::WindowResize&)> on_resize_;
        EventDispatcher<void(const events::WindowResizeEnd&)> on_resize_end_;
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
        return fmt::format_to(ctx.out(), "(event) OnWindowMove {{ position: {} }}", move.new_position);
    }
};

template<>
struct fmt::formatter<orion::events::WindowMoveEnd> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowMoveEnd& move, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowMoveEnd {{ final_position: {} }}", move.final_position);
    }
};

template<>
struct fmt::formatter<orion::events::WindowResize> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowResize& resize, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowResize {{ size: {} }}", resize.new_size);
    }
};

template<>
struct fmt::formatter<orion::events::WindowResizeEnd> : formatter<string_view> {
    template<typename FormatContext>
    auto format(const orion::events::WindowResizeEnd& resize, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(event) OnWindowResizeEnd {{ final_size: {} }}", resize.final_size);
    }
};
