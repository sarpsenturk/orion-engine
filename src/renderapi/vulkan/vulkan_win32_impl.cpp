#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>

#include "vulkan_error.h"
#include "vulkan_platform.h"

#include "win32/win32_window.h"

namespace orion
{
    VkSurfaceKHR create_platform_surface(VkInstance instance, const class Window* window)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        {
            const auto info = VkWin32SurfaceCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .hinstance = GetModuleHandle(nullptr),
                .hwnd = window->platform_window()->hwnd,
            };
            vk_assert(vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface));
        }
        return surface;
    }
} // namespace orion
