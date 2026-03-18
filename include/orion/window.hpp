#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>

namespace orion
{
    struct WindowDesc {
        const char* title;
        int width;
        int height;
    };

    struct OnWindowClose {
    };

    struct OnWindowMove {
        int xpos;
        int ypos;
    };

    struct OnWindowResize {
        int width;
        int height;
    };

    class WindowEvent
    {
    public:
        using EventVariant = std::variant<
            OnWindowClose,
            OnWindowMove,
            OnWindowResize>;

        WindowEvent(EventVariant payload)
            : payload_(payload)
        {
        }

        const EventVariant& payload() const { return payload_; }

    private:
        EventVariant payload_;
    };

    class Window
    {
    public:
        struct Impl;

        explicit Window(const WindowDesc& desc);
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = default;
        Window& operator=(Window&&) = default;
        ~Window();

        [[nodiscard]] Impl* impl() const noexcept { return impl_.get(); }

        [[nodiscard]] int width() const;
        [[nodiscard]] int height() const;
        [[nodiscard]] bool should_close() const;

        void poll_events();
        void set_event_callback(std::function<void(const WindowEvent&)> callback);

    private:
        std::unique_ptr<Impl> impl_;
    };

    std::string format_as(const OnWindowClose&);
    std::string format_as(const OnWindowResize& event);
    std::string format_as(const OnWindowMove& event);
} // namespace orion
