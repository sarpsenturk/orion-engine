#include "orion-core/input.h"

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

    void Keyboard::set_state(KeyCode key, KeyState state) noexcept
    {
        key_states_[static_cast<std::size_t>(key)] = state;
    }

    std::string format_as(KeyCode keycode) noexcept
    {
        switch (keycode) {
            case KeyCode::Unknown:
                break;
            case KeyCode::Backspace:
                return "Backspace";
            case KeyCode::Tab:
                return "Tab";
            case KeyCode::Enter:
                return "Enter";
            case KeyCode::Escape:
                return "Escape";
            case KeyCode::Space:
                return "Space";
            case KeyCode::Quote:
                return "Quote";
            case KeyCode::Comma:
                return "Comma";
            case KeyCode::Minus:
                return "Minus";
            case KeyCode::Period:
                return "Period";
            case KeyCode::Slash:
                return "Slash";
            case KeyCode::Semicolon:
                return "Semicolon ';'";
            case KeyCode::Equal:
                return "Equal '='";
            case KeyCode::LeftBracket:
                return "Left bracket '['";
            case KeyCode::Backslash:
                return "Backslash '\\'";
            case KeyCode::RightBracket:
                return "Right bracket ']'";
            case KeyCode::Backtick:
                return "Backtick '`'";
            case KeyCode::Delete:
                return "Delete";
            case KeyCode::Insert:
                return "Insert";
            case KeyCode::Home:
                return "Home";
            case KeyCode::End:
                return "End";
            case KeyCode::PageUp:
                return "Page Up";
            case KeyCode::PageDown:
                return "Page Down";
            case KeyCode::PrintScreen:
                return "Print Screen";
            case KeyCode::ScrollLock:
                return "Scroll Lock";
            case KeyCode::Pause:
                return "Pause";
            case KeyCode::Caps:
                return "Caps Lock";
            case KeyCode::Shift:
                return "Shift";
            case KeyCode::Control:
                return "Control";
            case KeyCode::Alt:
                return "Alt";
            case KeyCode::Command:
                return "Command";
            case KeyCode::NumDivide:
                return "Numpad Divide '/'";
            case KeyCode::NumMultiply:
                return "Numpad Multiply '*'";
            case KeyCode::NumMinus:
                return "Numpad Minus '-'";
            case KeyCode::NumPlus:
                return "Numpad Plus '+'";
            case KeyCode::NumEnter:
                return "Numpad Enter";
            case KeyCode::NumPeriod:
                return "Numpad Period";
            case KeyCode::NumLock:
                return "NumLock";
            case KeyCode::LeftArrow:
                return "Left Arrow";
            case KeyCode::UpArrow:
                return "Up Arrow";
            case KeyCode::RightArrow:
                return "Right Arrow";
            case KeyCode::DownArrow:
                return "Down Arrow";
            case KeyCode::Max:
                break;
            default:
                if (is_numeric_key(keycode) || is_char_key(keycode)) {
                    return fmt::format("Key {}", static_cast<char>(keycode));
                }
                if (is_numpad_key(keycode)) {
                    const auto offset = static_cast<int>(keycode) - static_cast<int>(KeyCode::Num0);
                    return fmt::format("Numpad {}", offset);
                }
                if (is_fn_key(keycode)) {
                    const auto offset = static_cast<int>(keycode) - static_cast<int>(KeyCode::F1);
                    return fmt::format("F{}", offset + 1);
                }
                break;
        }
        return "Unknown";
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
    } // namespace events
} // namespace orion
