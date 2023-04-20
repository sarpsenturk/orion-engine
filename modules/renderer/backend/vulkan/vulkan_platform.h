#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include <exception>           // std::exception
#include <orion-core/config.h> // orion::current_platform
#include <orion-core/window.h> // orion::Window

namespace orion::vulkan
{
    // Returns the platform specific surface extension
    // win32 -> VK_KHR_win32_surface, X11 -> VK_KHR_xlib_surface, etc.
    constexpr const char* platform_surface_ext() noexcept
    {
        if constexpr (current_platform == Platform::Windows) {
            return "VK_KHR_win32_surface";
        } else {
            throw std::exception("No supported Vulkan surface extension for current platform");
        }
    }

    UniqueVkSurfaceKHR create_surface(VkInstance instance, const Window& window);
} // namespace orion::vulkan
