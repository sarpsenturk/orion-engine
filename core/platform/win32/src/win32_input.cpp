#include "orion-win32/win32_input.h"

namespace orion
{
    KeyCode win32_vk_to_keycode(WPARAM wparam)
    {
        auto is_numeric_key = [](WPARAM wparam) { return (wparam >= 0x30) && (wparam <= 0x39); };
        auto is_char_key = [](WPARAM wparam) { return (wparam >= 0x41) && (wparam <= 0x5A); };
        auto is_numpad_key = [](WPARAM wparam) { return (wparam >= VK_NUMPAD0) && (wparam <= VK_NUMPAD9); };
        auto is_func = [](WPARAM wparam) { return (wparam >= VK_F1) && (wparam <= VK_F15); };
        auto calculate_from_offset = [](WPARAM wparam, KeyCode key_start, WPARAM wparam_start) {
            const auto key_first = static_cast<WPARAM>(key_start);
            const auto offset = wparam - wparam_start;
            return static_cast<KeyCode>(key_first + offset);
        };

        switch (wparam) {
            case VK_BACK:
                return KeyCode::Backspace;
            case VK_TAB:
                return KeyCode::Tab;
            case VK_RETURN:
                return KeyCode::Enter;
            case VK_SHIFT:
                return KeyCode::Shift;
            case VK_CONTROL:
                return KeyCode::Control;
            case VK_MENU:
                return KeyCode::Alt;
            case VK_PAUSE:
                return KeyCode::Pause;
            case VK_CAPITAL:
                return KeyCode::Caps;
            case VK_ESCAPE:
                return KeyCode::Escape;
            case VK_SPACE:
                return KeyCode::Space;
            case VK_PRIOR:
                return KeyCode::PageUp;
            case VK_NEXT:
                return KeyCode::PageDown;
            case VK_END:
                return KeyCode::End;
            case VK_HOME:
                return KeyCode::Home;
            case VK_LEFT:
                return KeyCode::LeftArrow;
            case VK_UP:
                return KeyCode::UpArrow;
            case VK_RIGHT:
                return KeyCode::RightArrow;
            case VK_DOWN:
                return KeyCode::DownArrow;
            case VK_SNAPSHOT:
                return KeyCode::PrintScreen;
            case VK_INSERT:
                return KeyCode::Insert;
            case VK_DELETE:
                return KeyCode::Delete;
            case VK_HELP:
                return KeyCode::F1;
            case VK_LWIN:
            case VK_RWIN:
                return KeyCode::Command;
            case VK_MULTIPLY:
                return KeyCode::NumMultiply;
            case VK_ADD:
                return KeyCode::NumPlus;
            case VK_SEPARATOR:
                return KeyCode::NumPeriod;
            case VK_SUBTRACT:
                return KeyCode::NumMinus;
            case VK_DECIMAL:
                return KeyCode::NumPeriod;
            case VK_DIVIDE:
                return KeyCode::NumDivide;
            case VK_NUMLOCK:
                return KeyCode::NumLock;
            case VK_SCROLL:
                return KeyCode::ScrollLock;
            case VK_OEM_1:
                return KeyCode::Semicolon;
            case VK_OEM_PLUS:
                return KeyCode::Equal;
            case VK_OEM_COMMA:
                return KeyCode::Comma;
            case VK_OEM_MINUS:
                return KeyCode::Minus;
            case VK_OEM_PERIOD:
                return KeyCode::Period;
            case VK_OEM_2:
                return KeyCode::Slash;
            case VK_OEM_3:
                return KeyCode::Backtick;
            case VK_OEM_4:
                return KeyCode::LeftBracket;
            case VK_OEM_5:
                return KeyCode::Backslash;
            case VK_OEM_6:
                return KeyCode::RightBracket;
            case VK_OEM_7:
                return KeyCode::Quote;
            default:
                // Alphanumeric characters match Windows virtual keys since
                // we both use their ASCII representations
                if (is_numeric_key(wparam) || is_char_key(wparam)) {
                    return static_cast<KeyCode>(wparam);
                }
                if (is_numpad_key(wparam)) {
                    return calculate_from_offset(wparam, KeyCode::Num0, VK_NUMPAD0);
                }
                if (is_func(wparam)) {
                    return calculate_from_offset(wparam, KeyCode::F1, VK_F1);
                }
                break;
        }
        return KeyCode::Unknown;
    }
} // namespace orion
