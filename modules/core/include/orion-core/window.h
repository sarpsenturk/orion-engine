#pragma once

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

    private:
        PlatformWindowPtr platform_window_;

        std::string name_;
        WindowPosition position_;
        WindowSize size_;
        bool should_close_ = false;
    };
} // namespace orion
