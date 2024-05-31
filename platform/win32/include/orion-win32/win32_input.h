#pragma once

#include "orion-platform/input.h"
#include "win32_platform.h"

namespace orion
{
    KeyCode win32_vk_to_keycode(WPARAM vk);
    int keycode_to_win32_vk(KeyCode keycode);
}
