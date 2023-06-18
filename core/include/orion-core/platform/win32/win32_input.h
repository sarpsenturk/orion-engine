#pragma once

#include "orion-core/input.h"
#include "win32_headers.h"

namespace orion
{
    KeyCode win32_vk_to_keycode(WPARAM wparam);
}
