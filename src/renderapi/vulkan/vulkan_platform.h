#pragma once

#include <volk.h>

namespace orion
{
    VkSurfaceKHR create_platform_surface(VkInstance instance, const class Window* window);
}
