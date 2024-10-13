#pragma once

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

    std::string format_as(const OnWindowClose&);
    std::string format_as(const OnWindowResize& resize);
    std::string format_as(const OnWindowMove& move);

    struct WindowEvent {
        using EventData = std::variant<
            std::monostate,
            OnWindowClose,
            OnWindowResize,
            OnWindowMove>;
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

        [[nodiscard]] const PlatformWindow* platform_window() const noexcept { return platform_window_.get(); }

    private:
        std::unique_ptr<PlatformWindow> platform_window_;
    };
} // namespace orion
