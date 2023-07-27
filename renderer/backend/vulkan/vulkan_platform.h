#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include "orion-core/config.h"
#include "orion-core/window.h"

#include <exception>

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

    UniqueVkSurfaceKHR create_surface(VkInstance instance, const Window* window);
} // namespace orion::vulkan
