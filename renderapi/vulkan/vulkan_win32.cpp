#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "vulkan_platform.h"

#include <orion-win32/win32_window.h>

namespace orion::vulkan
{
    VkSurfaceKHR create_platform_surface(VkInstance instance, const Window& window)
    {
        auto* platform_window = window.platform_window();
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        {
            const auto info = VkWin32SurfaceCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .hinstance = platform_window->hinstance(),
                .hwnd = platform_window->hwnd(),
            };
            vk_result_check(vkCreateWin32SurfaceKHR(instance, &info, alloc_callbacks(), &surface));
        }
        return surface;
    }
} // namespace orion::vulkan
