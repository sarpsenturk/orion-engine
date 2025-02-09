#include "orion/input.hpp"

#include "orion/assertion.hpp"

#include <type_traits>

namespace orion
{
    bool Keyboard::is_key_down(Keycode key) const
    {
        const auto index = static_cast<std::underlying_type_t<Keycode>>(key);
        ORION_ASSERT(0 <= index && index < keys_.size());
        return keys_[index] == KeyState::Down;
    }

    void Keyboard::set_key_down(Keycode key)
    {
        const auto index = static_cast<std::underlying_type_t<Keycode>>(key);
        ORION_ASSERT(0 <= index && index < keys_.size());
        keys_[index] = KeyState::Down;
    }

    void Keyboard::set_key_up(Keycode key)
    {
        const auto index = static_cast<std::underlying_type_t<Keycode>>(key);
        ORION_ASSERT(0 <= index && index < keys_.size());
        keys_[index] = KeyState::Up;
    }

    void Mouse::set_button_down(MouseButton button)
    {
        const auto index = static_cast<std::underlying_type_t<MouseButton>>(button);
        ORION_ASSERT(0 <= index && index < buttons_.size());
        buttons_[index] = true;
    }

    void Mouse::set_button_up(MouseButton button)
    {
        const auto index = static_cast<std::underlying_type_t<MouseButton>>(button);
        ORION_ASSERT(0 <= index && index < buttons_.size());
        buttons_[index] = false;
    }

    bool Mouse::is_button_down(MouseButton button) const
    {
        const auto index = static_cast<std::underlying_type_t<MouseButton>>(button);
        ORION_ASSERT(0 <= index && index < buttons_.size());
        return buttons_[index];
    }

    const char* format_as(Keycode keycode)
    {
        switch (keycode) {
            case Keycode::Unknown:
                return "Unknown";
            case Keycode::Backspace:
                return "Backspace";
            case Keycode::Tab:
                return "Tab";
            case Keycode::Enter:
                return "Enter";
            case Keycode::Escape:
                return "Escape";
            case Keycode::Space:
                return "Space";
            case Keycode::Quote:
                return "Quote";
            case Keycode::Comma:
                return "Comma";
            case Keycode::Minus:
                return "Minus";
            case Keycode::Period:
                return "Period";
            case Keycode::Slash:
                return "Slash";
            case Keycode::Alpha0:
                return "Alpha0";
            case Keycode::Alpha1:
                return "Alpha1";
            case Keycode::Alpha2:
                return "Alpha2";
            case Keycode::Alpha3:
                return "Alpha3";
            case Keycode::Alpha4:
                return "Alpha4";
            case Keycode::Alpha5:
                return "Alpha5";
            case Keycode::Alpha6:
                return "Alpha6";
            case Keycode::Alpha7:
                return "Alpha7";
            case Keycode::Alpha8:
                return "Alpha8";
            case Keycode::Alpha9:
                return "Alpha9";
            case Keycode::Semicolon:
                return "Semicolon";
            case Keycode::Equal:
                return "Equal";
            case Keycode::KeyA:
                return "KeyA";
            case Keycode::KeyB:
                return "KeyB";
            case Keycode::KeyC:
                return "KeyC";
            case Keycode::KeyD:
                return "KeyD";
            case Keycode::KeyE:
                return "KeyE";
            case Keycode::KeyF:
                return "KeyF";
            case Keycode::KeyG:
                return "KeyG";
            case Keycode::KeyH:
                return "KeyH";
            case Keycode::KeyI:
                return "KeyI";
            case Keycode::KeyJ:
                return "KeyJ";
            case Keycode::KeyK:
                return "KeyK";
            case Keycode::KeyL:
                return "KeyL";
            case Keycode::KeyM:
                return "KeyM";
            case Keycode::KeyN:
                return "KeyN";
            case Keycode::KeyO:
                return "KeyO";
            case Keycode::KeyP:
                return "KeyP";
            case Keycode::KeyQ:
                return "KeyQ";
            case Keycode::KeyR:
                return "KeyR";
            case Keycode::KeyS:
                return "KeyS";
            case Keycode::KeyT:
                return "KeyT";
            case Keycode::KeyU:
                return "KeyU";
            case Keycode::KeyV:
                return "KeyV";
            case Keycode::KeyW:
                return "KeyW";
            case Keycode::KeyX:
                return "KeyX";
            case Keycode::KeyY:
                return "KeyY";
            case Keycode::KeyZ:
                return "KeyZ";
            case Keycode::LeftBracket:
                return "LeftBracket";
            case Keycode::Backslash:
                return "Backslash";
            case Keycode::RightBracket:
                return "RightBracket";
            case Keycode::Backtick:
                return "Backtick";
            case Keycode::Delete:
                return "Delete";
            case Keycode::Insert:
                return "Insert";
            case Keycode::Home:
                return "Home";
            case Keycode::End:
                return "End";
            case Keycode::PageUp:
                return "PageUp";
            case Keycode::PageDown:
                return "PageDown";
            case Keycode::PrintScreen:
                return "PrintScreen";
            case Keycode::ScrollLock:
                return "ScrollLock";
            case Keycode::Pause:
                return "Pause";
            case Keycode::F1:
                return "F1";
            case Keycode::F2:
                return "F2";
            case Keycode::F3:
                return "F3";
            case Keycode::F4:
                return "F4";
            case Keycode::F5:
                return "F5";
            case Keycode::F6:
                return "F6";
            case Keycode::F7:
                return "F7";
            case Keycode::F8:
                return "F8";
            case Keycode::F9:
                return "F9";
            case Keycode::F10:
                return "F10";
            case Keycode::F11:
                return "F11";
            case Keycode::F12:
                return "F12";
            case Keycode::F13:
                return "F13";
            case Keycode::F14:
                return "F14";
            case Keycode::F15:
                return "F15";
            case Keycode::FMax:
                return "FMax";
            case Keycode::Caps:
                return "Caps";
            case Keycode::Shift:
                return "Shift";
            case Keycode::Control:
                return "Control";
            case Keycode::Alt:
                return "Alt";
            case Keycode::Command:
                return "Command";
            case Keycode::Num0:
                return "Num0";
            case Keycode::Num1:
                return "Num1";
            case Keycode::Num2:
                return "Num2";
            case Keycode::Num3:
                return "Num3";
            case Keycode::Num4:
                return "Num4";
            case Keycode::Num5:
                return "Num5";
            case Keycode::Num6:
                return "Num6";
            case Keycode::Num7:
                return "Num7";
            case Keycode::Num8:
                return "Num8";
            case Keycode::Num9:
                return "Num9";
            case Keycode::NumDivide:
                return "NumDivide";
            case Keycode::NumMultiply:
                return "NumMultiply";
            case Keycode::NumMinus:
                return "NumMinus";
            case Keycode::NumPlus:
                return "NumPlus";
            case Keycode::NumEnter:
                return "NumEnter";
            case Keycode::NumPeriod:
                return "NumPeriod";
            case Keycode::NumLock:
                return "NumLock";
            case Keycode::LeftArrow:
                return "LeftArrow";
            case Keycode::UpArrow:
                return "UpArrow";
            case Keycode::RightArrow:
                return "RightArrow";
            case Keycode::DownArrow:
                return "DownArrow";
            case Keycode::Max:
                return "Max";
        }
        unreachable();
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
        }
        unreachable();
    }
} // namespace orion
