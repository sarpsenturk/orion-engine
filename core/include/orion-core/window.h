#pragma once

#include "orion-platform/window.h"

#include <cstdint>
#include <string>

namespace spdlog
{
    class logger;
}

namespace orion
{
    namespace defaults
    {
        inline constexpr auto window_position = WindowPosition{0, 0};
        inline constexpr auto window_size = WindowSize{800, 600};
    } // namespace defaults

    class Window
    {
    public:
        explicit Window(WindowDesc window_desc);

        WindowEvent poll_event();

        [[nodiscard]] auto* platform_window() const noexcept { return platform_window_.get(); }
        [[nodiscard]] auto& name() const noexcept { return name_; }
        [[nodiscard]] auto& position() const noexcept { return position_; }
        [[nodiscard]] auto& size() const noexcept { return size_; }
        [[nodiscard]] auto aspect_ratio() const noexcept { return static_cast<float>(size_.x()) / size_.y(); }

        [[nodiscard]] bool is_maximized() const noexcept;
        [[nodiscard]] bool is_minimized() const noexcept;
        [[nodiscard]] bool has_focus() const noexcept;

    private:
        static spdlog::logger* logger();

        void on_resize(const WindowResize* resize);
        void on_move(const WindowMove* move);
        void on_activate();
        void on_deactivate();

        PlatformWindowPtr platform_window_;

        // Window information
        std::string name_;
        WindowPosition position_;
        WindowSize size_;

        // Window state is packed into an integer
        //  bit 0: Maximized
        //  bit 1: Minimized
        //  bit 2: Focused
        std::uint8_t state_ = 0x00;
    };
} // namespace orion
