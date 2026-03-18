#pragma once

#include "orion/event.hpp"

#include <memory>
#include <string>

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

        Event<OnWindowClose>& on_window_close();
        Event<OnWindowMove>& on_window_move();
        Event<OnWindowResize>& on_window_resize();

    private:
        std::unique_ptr<Impl> impl_;
    };

    std::string format_as(const OnWindowClose&);
    std::string format_as(const OnWindowResize& event);
    std::string format_as(const OnWindowMove& event);
} // namespace orion
