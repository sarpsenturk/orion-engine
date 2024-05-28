#pragma once

#include "orion-math/vector/vector2.h"

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

    struct WindowResize {
        WindowSize size;
    };

    struct WindowMove {
        WindowPosition position;
    };

    using WindowEventData = std::variant<std::monostate,
                                         WindowClose,
                                         WindowResize,
                                         WindowMove>;

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

    namespace platform
    {
        PlatformWindow* create_window(const WindowDesc& window_desc);
        void destroy_window(PlatformWindow* platform_window);
        WindowEvent poll_event_window(PlatformWindow* platform_window);
    } // namespace platform

    // unique_ptr to platform specific implementation
    using PlatformWindowPtr = std::unique_ptr<PlatformWindow, decltype(&platform::destroy_window)>;
} // namespace orion
