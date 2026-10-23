#pragma once

#include "input.hpp"

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
        explicit Window(const WindowDesc& desc);
        Window(const Window&)            = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&& other) noexcept;
        Window& operator=(Window&& other) noexcept;
        ~Window();

        [[nodiscard]] void* handle() const noexcept { return m_handle; }
        [[nodiscard]] int width() const noexcept { return m_width; }
        [[nodiscard]] int height() const noexcept { return m_height; }
        [[nodiscard]] float aspect_ratio() const noexcept { return static_cast<float>(m_width) / m_height; }
        [[nodiscard]] bool should_close() const noexcept;

        [[nodiscard]] bool is_key_pressed(Keycode key) const noexcept;
        [[nodiscard]] bool is_mouse_button_pressed(MouseButton button) const noexcept;
        [[nodiscard]] MousePosition mouse_position() const noexcept;

    private:
        void* m_handle;
        int m_width;
        int m_height;
    };
} // namespace orion
