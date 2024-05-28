#include "orion-core/input.h"

#include "orion-utils/assertion.h"

#include <fmt/format.h>

namespace orion
{
    Keyboard::Keyboard()
    {
        on_key_press_.subscribe([this](auto& key_press) { set_state(key_press.key, KeyState::Down); });
        on_key_release_.subscribe([this](auto& key_release) { set_state(key_release.key, KeyState::Up); });
        on_key_repeat_.subscribe([this](auto& key_repeat) { set_state(key_repeat.key, KeyState::Repeated); });
    }

    bool Keyboard::key_pressed(KeyCode key) const noexcept
    {
        auto state = key_state(key);
        return state == KeyState::Down || state == KeyState::Repeated;
    }

    std::size_t Keyboard::get_key_index(KeyCode key) const noexcept
    {
        const auto key_index = static_cast<std::size_t>(key);
        ORION_EXPECTS(key_index < max_key);
        return key_index;
    }

    void Keyboard::set_state(KeyCode key, KeyState state) noexcept
    {
        key_states_[get_key_index(key)] = state;
    }

    Mouse::Mouse()
    {
        on_move_.subscribe([this](const auto& move) { set_position(move.position); });
        on_button_down_.subscribe([this](const auto& button_down) { set_button_state(button_down.button, true); });
        on_button_up_.subscribe([this](const auto& button_up) { set_button_state(button_up.button, false); });
    }

    std::size_t Mouse::get_button_index(MouseButton button) const noexcept
    {
        const auto button_index = static_cast<std::size_t>(button);
        ORION_EXPECTS(button_index < max_mouse_button);
        return button_index;
    }

    void Mouse::set_button_state(MouseButton button, bool is_down)
    {
        button_states_[get_button_index(button)] = is_down;
    }

    void Mouse::set_position(MousePosition position)
    {
        position_ = position;
    }

    namespace events
    {
        std::string format_as(const KeyRelease& key_release)
        {
            return fmt::format("(event) OnKeyRelease {{ key: {} }}", key_release.key);
        }

        std::string format_as(const KeyPress& key_press)
        {
            return fmt::format("(event) OnKeyPress {{ key: {} }}", key_press.key);
        }

        std::string format_as(const KeyRepeat& key_repeat)
        {
            return fmt::format("(event) OnKeyRepeat {{ key: {} }}", key_repeat.key);
        }

        std::string events::format_as(const MouseMove& mouse_move)
        {
            return fmt::format("(event) OnMouseMove {{ position: {} }}", mouse_move.position);
        }

        std::string events::format_as(const MouseButtonDown& mouse_button_down)
        {
            return fmt::format("(event) OnMouseButtonDown {{ button: {}, position: {} }}",
                               mouse_button_down.button, mouse_button_down.position);
        }

        std::string events::format_as(const MouseButtonUp& mouse_button_up)
        {
            return fmt::format("(event) OnMouseButtonUp {{ button: {}, position: {} }}",
                               mouse_button_up.button, mouse_button_up.position);
        }

        std::string events::format_as(const MouseScroll& mouse_scroll)
        {
            return fmt::format("(event) OnMouseScroll {{ delta: {}, position: {} }}",
                               mouse_scroll.delta, mouse_scroll.position);
        }
    } // namespace events
} // namespace orion
