#include "orion-platform/input.h"

namespace orion
{
    const char* format_as(KeyCode keycode)
    {
        switch (keycode) {
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

            case KeyCode::Alpha0:
                return "Alpha0";
            case KeyCode::Alpha1:
                return "Alpha1";
            case KeyCode::Alpha2:
                return "Alpha2";
            case KeyCode::Alpha3:
                return "Alpha3";
            case KeyCode::Alpha4:
                return "Alpha4";
            case KeyCode::Alpha5:
                return "Alpha5";
            case KeyCode::Alpha6:
                return "Alpha6";
            case KeyCode::Alpha7:
                return "Alpha7";
            case KeyCode::Alpha8:
                return "Alpha8";
            case KeyCode::Alpha9:
                return "Alpha9";

            case KeyCode::Semicolon:
                return "Semicolon";
            case KeyCode::Equal:
                return "Equal";

            case KeyCode::KeyA:
                return "KeyA";
            case KeyCode::KeyB:
                return "KeyB";
            case KeyCode::KeyC:
                return "KeyC";
            case KeyCode::KeyD:
                return "KeyD";
            case KeyCode::KeyE:
                return "KeyE";
            case KeyCode::KeyF:
                return "KeyF";
            case KeyCode::KeyG:
                return "KeyG";
            case KeyCode::KeyH:
                return "KeyH";
            case KeyCode::KeyI:
                return "KeyI";
            case KeyCode::KeyJ:
                return "KeyJ";
            case KeyCode::KeyK:
                return "KeyK";
            case KeyCode::KeyL:
                return "KeyL";
            case KeyCode::KeyM:
                return "KeyM";
            case KeyCode::KeyN:
                return "KeyN";
            case KeyCode::KeyO:
                return "KeyO";
            case KeyCode::KeyP:
                return "KeyP";
            case KeyCode::KeyQ:
                return "KeyQ";
            case KeyCode::KeyR:
                return "KeyR";
            case KeyCode::KeyS:
                return "KeyS";
            case KeyCode::KeyT:
                return "KeyT";
            case KeyCode::KeyU:
                return "KeyU";
            case KeyCode::KeyV:
                return "KeyV";
            case KeyCode::KeyW:
                return "KeyW";
            case KeyCode::KeyX:
                return "KeyX";
            case KeyCode::KeyY:
                return "KeyY";
            case KeyCode::KeyZ:
                return "KeyZ";

            case KeyCode::LeftBracket:
                return "LeftBracket";
            case KeyCode::Backslash:
                return "Backslash";
            case KeyCode::RightBracket:
                return "RightBracket";
            case KeyCode::Backtick:
                return "Backtick";
            case KeyCode::Delete:
                return "Delete";

            case KeyCode::Insert:
                return "Insert";
            case KeyCode::Home:
                return "Home";
            case KeyCode::End:
                return "End";
            case KeyCode::PageUp:
                return "PageUp";
            case KeyCode::PageDown:
                return "PageDown";

            case KeyCode::PrintScreen:
                return "PrintScreen";
            case KeyCode::ScrollLock:
                return "ScrollLock";
            case KeyCode::Pause:
                return "Pause";

            case KeyCode::F1:
                return "F1";
            case KeyCode::F2:
                return "F2";
            case KeyCode::F3:
                return "F3";
            case KeyCode::F4:
                return "F4";
            case KeyCode::F5:
                return "F5";
            case KeyCode::F6:
                return "F6";
            case KeyCode::F7:
                return "F7";
            case KeyCode::F8:
                return "F8";
            case KeyCode::F9:
                return "F9";
            case KeyCode::F10:
                return "F10";
            case KeyCode::F11:
                return "F11";
            case KeyCode::F12:
                return "F12";
            case KeyCode::F13:
                return "F13";
            case KeyCode::F14:
                return "F14";
            case KeyCode::F15:
                return "F15";
            case KeyCode::FMax:
                return "FMax";

            case KeyCode::Caps:
                return "Caps";
            case KeyCode::Shift:
                return "Shift";
            case KeyCode::Control:
                return "Control";
            case KeyCode::Alt:
                return "Alt";
            case KeyCode::Command:
                return "Command";

            case KeyCode::Num0:
                return "Num0";
            case KeyCode::Num1:
                return "Num1";
            case KeyCode::Num2:
                return "Num2";
            case KeyCode::Num3:
                return "Num3";
            case KeyCode::Num4:
                return "Num4";
            case KeyCode::Num5:
                return "Num5";
            case KeyCode::Num6:
                return "Num6";
            case KeyCode::Num7:
                return "Num7";
            case KeyCode::Num8:
                return "Num8";
            case KeyCode::Num9:
                return "Num9";
            case KeyCode::NumDivide:
                return "NumDivide";
            case KeyCode::NumMultiply:
                return "NumMultiply";
            case KeyCode::NumMinus:
                return "NumMinus";
            case KeyCode::NumPlus:
                return "NumPlus";
            case KeyCode::NumEnter:
                return "NumEnter";
            case KeyCode::NumPeriod:
                return "NumPeriod";
            case KeyCode::NumLock:
                return "NumLock";

            case KeyCode::LeftArrow:
                return "LeftArrow";
            case KeyCode::UpArrow:
                return "UpArrow";
            case KeyCode::RightArrow:
                return "RightArrow";
            case KeyCode::DownArrow:
                return "DownArrow";
            default:
                return "Unknown";
        }
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
            default:
                return "Unknown";
        }
    }
} // namespace orion
