#pragma once

#include "orion/input.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

namespace orion
{
    inline constexpr auto window_position_default = INT32_MAX;

    // struct to describe how a window should be created
    struct WindowDesc {
        const char* title = "Orion Window";
        std::uint32_t width = UINT32_MAX;
        std::uint32_t height = UINT32_MAX;
        std::int32_t xpos = window_position_default;
        std::int32_t ypos = window_position_default;
    };

    // Window events begin with On*

    // Sent to the window when the user closes the window
    struct OnWindowClose {
    };

    // Sent to the window when the window is resized
    struct OnWindowResize {
        std::uint32_t width;
        std::uint32_t height;
    };

    // Sent to the window when the window position changed
    struct OnWindowMove {
        std::int32_t xpos;
        std::int32_t ypos;
    };

    // Sent to the window when a key is pressed
    struct OnKeyDown {
        Keycode keycode;
    };

    // Sent to the window when a key is released
    struct OnKeyUp {
        Keycode keycode;
    };

    // Sent to the window when the mouse is moved
    struct OnMouseMove {
        std::int32_t xpos;
        std::int32_t ypos;
    };

    // Sent to the window when a mouse button is pressed
    struct OnMouseButtonDown {
        MouseButton button;
        std::int32_t xpos;
        std::int32_t ypos;
    };

    // Sent to the window when a mouse button is released
    struct OnMouseButtonUp {
        MouseButton button;
        std::int32_t xpos;
        std::int32_t ypos;
    };

    // Sent to the window when the mouse wheel is rotated
    struct OnMouseScroll {
        std::int32_t scroll;
        std::int32_t xpos;
        std::int32_t ypos;
    };

    std::string format_as(const OnWindowClose&);
    std::string format_as(const OnWindowResize& resize);
    std::string format_as(const OnWindowMove& move);
    std::string format_as(const OnKeyDown& keydown);
    std::string format_as(const OnKeyUp& keyup);
    std::string format_as(const OnMouseMove& mousemove);
    std::string format_as(const OnMouseButtonDown& mousebuttondown);
    std::string format_as(const OnMouseButtonUp& mousebuttonup);
    std::string format_as(const OnMouseScroll& mousescroll);

    struct WindowEvent {
        using EventData = std::variant<
            std::monostate,

            OnWindowClose,
            OnWindowResize,
            OnWindowMove,

            OnKeyDown,
            OnKeyUp,

            OnMouseMove,
            OnMouseButtonDown,
            OnMouseButtonUp,
            OnMouseScroll>;
        EventData data;

        WindowEvent& operator=(EventData event)
        {
            data = std::move(event);
            return *this;
        }

        explicit(false) operator bool() const noexcept { return !std::holds_alternative<std::monostate>(data); }

        template<typename T>
        [[nodiscard]] bool is() const noexcept
        {
            return std::holds_alternative<T>(data);
        }

        template<typename T>
        [[nodiscard]] const T* as() const noexcept
        {
            return std::get_if<T>(&data);
        }
    };

    std::string format_as(const WindowEvent& event);

    // Represent the platform implementation internals of a Window
    struct PlatformWindow;

    struct WindowCreateError final : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class Window
    {
    public:
        explicit Window(const WindowDesc& desc);
        Window(const Window&) = delete;
        Window(Window&&) noexcept;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) noexcept;
        ~Window();

        WindowEvent poll_event();

        [[nodiscard]] const Keyboard& keyboard() const { return keyboard_; }
        [[nodiscard]] const Mouse& mouse() const { return mouse_; }

        [[nodiscard]] const PlatformWindow* platform_window() const noexcept { return platform_window_.get(); }

    private:
        WindowEvent poll_event_impl();

        std::unique_ptr<PlatformWindow> platform_window_;

        // Input devices

        Keyboard keyboard_;
        Mouse mouse_;
    };
} // namespace orion
