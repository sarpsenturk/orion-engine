#pragma once

#include "orion-platform/input.h"

#include "orion-math/vector/vector2.h"

#include <cstdint>
#include <string>
#include <variant>

namespace orion
{
    // Forward declare
    class PlatformWindow;

    // Window data type aliases
    using WindowPosition = Vector2_i;
    using WindowSize = Vector2_u;

    // Window create info
    struct WindowDesc {
        std::string name;
        WindowPosition position;
        WindowSize size;
    };

    struct WindowClose {
    };

    inline constexpr std::uint8_t window_maximized = 0x1;
    inline constexpr std::uint8_t window_minimized = 0x2;
    inline constexpr std::uint8_t window_focused = 0x4;

    struct WindowResize {
        WindowSize size;
        std::uint8_t type;
    };

    struct WindowMove {
        WindowPosition position;
    };

    struct WindowActivate {
    };

    struct WindowDeactivate {
    };

    struct KeyDown {
        KeyCode key;
    };

    struct KeyUp {
        KeyCode key;
    };

    struct KeyRepeat {
        KeyCode key;
    };

    struct MouseButtonDown {
        MouseButton button;
        MousePosition position;
    };

    struct MouseButtonUp {
        MouseButton button;
        MousePosition position;
    };

    struct MouseMove {
        MousePosition position;
    };

    struct MouseScroll {
        int delta;
        MousePosition position;
    };

    using WindowEventData = std::variant<std::monostate,
                                         WindowClose,
                                         WindowResize,
                                         WindowMove,
                                         WindowActivate,
                                         WindowDeactivate,
                                         KeyDown,
                                         KeyUp,
                                         KeyRepeat,
                                         MouseButtonDown,
                                         MouseButtonUp,
                                         MouseMove,
                                         MouseScroll>;

    struct WindowEvent {
        WindowEventData data = std::monostate{};

        [[nodiscard]] bool is_empty() const noexcept { return std::holds_alternative<std::monostate>(data); }
        explicit(false) operator bool() const noexcept { return !is_empty(); }

        WindowEvent& operator=(WindowEventData new_data)
        {
            data = std::move(new_data);
            return *this;
        }

        template<typename Expected>
        Expected* get_if() noexcept
        {
            return std::get_if<Expected>(&data);
        }

        template<typename Expected>
        const Expected* get_if() const noexcept
        {
            return std::get_if<Expected>(&data);
        }
    };

    std::string format_as(const WindowClose& event);
    std::string format_as(const WindowResize& event);
    std::string format_as(const WindowMove& event);
    std::string format_as(const WindowActivate& event);
    std::string format_as(const WindowDeactivate& event);
    std::string format_as(const KeyDown& event);
    std::string format_as(const KeyUp& event);
    std::string format_as(const KeyRepeat& event);
    std::string format_as(const MouseButtonDown& event);
    std::string format_as(const MouseButtonUp& event);
    std::string format_as(const MouseMove& event);
    std::string format_as(const MouseScroll& event);
    std::string format_as(const WindowEvent& event);

    namespace platform
    {
        PlatformWindow* create_window(const WindowDesc& window_desc);
        void destroy_window(PlatformWindow* platform_window);
        WindowEvent poll_event_window(PlatformWindow* platform_window);
    } // namespace platform

    // unique_ptr to platform specific implementation
    using PlatformWindowPtr = std::unique_ptr<PlatformWindow, decltype(&platform::destroy_window)>;
} // namespace orion
