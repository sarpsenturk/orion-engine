#pragma once

#include <memory>

namespace orion
{
    struct WindowDesc {
        const char* title;
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

    private:
        std::unique_ptr<Impl> impl_;
    };
} // namespace orion
