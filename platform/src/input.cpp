#include "orion-platform/input.h"

#include "orion-utils/assertion.h"

namespace orion
{
    std::string format_as(KeyCode keycode)
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

    const char* format_as(MouseButton mouse_button)
    {
        switch (mouse_button) {
            case MouseButton::Left:
                return "Left";
            case MouseButton::Right:
                return "Right";
            case MouseButton::Middle:
                return "Middle";
            case MouseButton::X1:
                return "X1";
            case MouseButton::X2:
                return "X2";
            case MouseButton::Max:
                break;
        }
        unreachable();
    }
} // namespace orion
