#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "vulkan_platform.h"

#include <orion-core/platform/win32/win32_window.h>

namespace orion::vulkan
{
    UniqueVkSurfaceKHR create_surface(VkInstance instance, const Window& window)
    {
        auto* platform_window = window.platform_window();
        const VkWin32SurfaceCreateInfoKHR info{
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .hinstance = platform_window->hinstance(),
            .hwnd = platform_window->hwnd(),
        };
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        vk_result_check(vkCreateWin32SurfaceKHR(instance, &info, alloc_callbacks(), &surface));
        return UniqueVkSurfaceKHR{surface, {.instance = instance}};
    }
} // namespace orion::vulkan
