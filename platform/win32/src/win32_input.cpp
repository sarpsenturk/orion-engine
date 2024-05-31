#include "orion-win32/win32_input.h"

#include "orion-utils/type.h"

namespace orion
{
    KeyCode win32_vk_to_keycode(WPARAM vk)
    {
        if (vk >= '0' && vk <= '9') {
            return KeyCode{to_underlying(KeyCode::Alpha0) + (static_cast<char>(vk) - '0')};
        }
        if (vk >= 'A' && vk <= 'Z') {
            return KeyCode{to_underlying(KeyCode::KeyA) + (static_cast<int>(vk) - 'A')};
        }

        switch (vk) {
            case VK_BACK:
                return KeyCode::Backspace;
            case VK_TAB:
                return KeyCode::Tab;
            case VK_RETURN:
                return KeyCode::Enter;
            case VK_ESCAPE:
                return KeyCode::Escape;
            case VK_SPACE:
                return KeyCode::Space;
            case VK_OEM_7:
                return KeyCode::Quote;
            case VK_OEM_COMMA:
                return KeyCode::Comma;
            case VK_OEM_MINUS:
                return KeyCode::Minus;
            case VK_OEM_PERIOD:
                return KeyCode::Period;
            case VK_OEM_2:
                return KeyCode::Slash;
            case VK_OEM_1:
                return KeyCode::Semicolon;
            case VK_OEM_PLUS:
                return KeyCode::Equal;
            case VK_OEM_4:
                return KeyCode::LeftBracket;
            case VK_OEM_5:
                return KeyCode::Backslash;
            case VK_OEM_6:
                return KeyCode::RightBracket;
            case VK_OEM_3:
                return KeyCode::Backtick;
            case VK_DELETE:
                return KeyCode::Delete;
            case VK_INSERT:
                return KeyCode::Insert;
            case VK_HOME:
                return KeyCode::Home;
            case VK_END:
                return KeyCode::End;
            case VK_PRIOR:
                return KeyCode::PageUp;
            case VK_NEXT:
                return KeyCode::PageDown;
            case VK_SNAPSHOT:
                return KeyCode::PrintScreen;
            case VK_SCROLL:
                return KeyCode::ScrollLock;
            case VK_PAUSE:
                return KeyCode::Pause;
            case VK_F1:
                return KeyCode::F1;
            case VK_F2:
                return KeyCode::F2;
            case VK_F3:
                return KeyCode::F3;
            case VK_F4:
                return KeyCode::F4;
            case VK_F5:
                return KeyCode::F5;
            case VK_F6:
                return KeyCode::F6;
            case VK_F7:
                return KeyCode::F7;
            case VK_F8:
                return KeyCode::F8;
            case VK_F9:
                return KeyCode::F9;
            case VK_F10:
                return KeyCode::F10;
            case VK_F11:
                return KeyCode::F11;
            case VK_F12:
                return KeyCode::F12;
            case VK_CAPITAL:
                return KeyCode::Caps;
            case VK_SHIFT:
                return KeyCode::Shift;
            case VK_CONTROL:
                return KeyCode::Control;
            case VK_MENU:
                return KeyCode::Alt;
            case VK_LWIN:
                return KeyCode::Command; // Usually maps to the left Windows key
            case VK_NUMPAD0:
                return KeyCode::Num0;
            case VK_NUMPAD1:
                return KeyCode::Num1;
            case VK_NUMPAD2:
                return KeyCode::Num2;
            case VK_NUMPAD3:
                return KeyCode::Num3;
            case VK_NUMPAD4:
                return KeyCode::Num4;
            case VK_NUMPAD5:
                return KeyCode::Num5;
            case VK_NUMPAD6:
                return KeyCode::Num6;
            case VK_NUMPAD7:
                return KeyCode::Num7;
            case VK_NUMPAD8:
                return KeyCode::Num8;
            case VK_NUMPAD9:
                return KeyCode::Num9;
            case VK_DIVIDE:
                return KeyCode::NumDivide;
            case VK_MULTIPLY:
                return KeyCode::NumMultiply;
            case VK_SUBTRACT:
                return KeyCode::NumMinus;
            case VK_ADD:
                return KeyCode::NumPlus;
            case VK_DECIMAL:
                return KeyCode::NumPeriod;
            case VK_NUMLOCK:
                return KeyCode::NumLock;
            case VK_LEFT:
                return KeyCode::LeftArrow;
            case VK_UP:
                return KeyCode::UpArrow;
            case VK_RIGHT:
                return KeyCode::RightArrow;
            case VK_DOWN:
                return KeyCode::DownArrow;
            default:
                return KeyCode::Unknown; // For undefined keys
        }
    }

    int keycode_to_win32_vk(KeyCode keycode)
    {
        if ((keycode >= KeyCode::Alpha0 && keycode <= KeyCode::Alpha9) ||
            (keycode >= KeyCode::KeyA && keycode <= KeyCode::KeyZ)) {
            return static_cast<int>(keycode);
        }

        switch (keycode) {
            case KeyCode::Backspace:
                return VK_BACK;
            case KeyCode::Tab:
                return VK_TAB;
            case KeyCode::Enter:
                return VK_RETURN;
            case KeyCode::Escape:
                return VK_ESCAPE;
            case KeyCode::Space:
                return VK_SPACE;
            case KeyCode::Quote:
                return VK_OEM_7;
            case KeyCode::Comma:
                return VK_OEM_COMMA;
            case KeyCode::Minus:
                return VK_OEM_MINUS;
            case KeyCode::Period:
                return VK_OEM_PERIOD;
            case KeyCode::Slash:
                return VK_OEM_2;
            case KeyCode::Semicolon:
                return VK_OEM_1;
            case KeyCode::Equal:
                return VK_OEM_PLUS;
            case KeyCode::LeftBracket:
                return VK_OEM_4;
            case KeyCode::Backslash:
                return VK_OEM_5;
            case KeyCode::RightBracket:
                return VK_OEM_6;
            case KeyCode::Backtick:
                return VK_OEM_3;
            case KeyCode::Delete:
                return VK_DELETE;
            case KeyCode::Insert:
                return VK_INSERT;
            case KeyCode::Home:
                return VK_HOME;
            case KeyCode::End:
                return VK_END;
            case KeyCode::PageUp:
                return VK_PRIOR;
            case KeyCode::PageDown:
                return VK_NEXT;
            case KeyCode::PrintScreen:
                return VK_SNAPSHOT;
            case KeyCode::ScrollLock:
                return VK_SCROLL;
            case KeyCode::Pause:
                return VK_PAUSE;
            case KeyCode::F1:
                return VK_F1;
            case KeyCode::F2:
                return VK_F2;
            case KeyCode::F3:
                return VK_F3;
            case KeyCode::F4:
                return VK_F4;
            case KeyCode::F5:
                return VK_F5;
            case KeyCode::F6:
                return VK_F6;
            case KeyCode::F7:
                return VK_F7;
            case KeyCode::F8:
                return VK_F8;
            case KeyCode::F9:
                return VK_F9;
            case KeyCode::F10:
                return VK_F10;
            case KeyCode::F11:
                return VK_F11;
            case KeyCode::F12:
                return VK_F12;
            case KeyCode::Caps:
                return VK_CAPITAL;
            case KeyCode::Shift:
                return VK_SHIFT;
            case KeyCode::Control:
                return VK_CONTROL;
            case KeyCode::Alt:
                return VK_MENU;
            case KeyCode::Command:
                return VK_LWIN; // Usually maps to the left Windows key
            case KeyCode::Num0:
                return VK_NUMPAD0;
            case KeyCode::Num1:
                return VK_NUMPAD1;
            case KeyCode::Num2:
                return VK_NUMPAD2;
            case KeyCode::Num3:
                return VK_NUMPAD3;
            case KeyCode::Num4:
                return VK_NUMPAD4;
            case KeyCode::Num5:
                return VK_NUMPAD5;
            case KeyCode::Num6:
                return VK_NUMPAD6;
            case KeyCode::Num7:
                return VK_NUMPAD7;
            case KeyCode::Num8:
                return VK_NUMPAD8;
            case KeyCode::Num9:
                return VK_NUMPAD9;
            case KeyCode::NumDivide:
                return VK_DIVIDE;
            case KeyCode::NumMultiply:
                return VK_MULTIPLY;
            case KeyCode::NumMinus:
                return VK_SUBTRACT;
            case KeyCode::NumPlus:
                return VK_ADD;
            case KeyCode::NumEnter:
                return VK_RETURN; // VK_SEPARATOR can also be used
            case KeyCode::NumPeriod:
                return VK_DECIMAL;
            case KeyCode::NumLock:
                return VK_NUMLOCK;
            case KeyCode::LeftArrow:
                return VK_LEFT;
            case KeyCode::UpArrow:
                return VK_UP;
            case KeyCode::RightArrow:
                return VK_RIGHT;
            case KeyCode::DownArrow:
                return VK_DOWN;
            default:
                return -1; // For KeyCode::Unknown and other undefined keys
        }
    }
} // namespace orion
