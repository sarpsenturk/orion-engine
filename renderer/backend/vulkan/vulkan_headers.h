#pragma once

#include "orion-core/defs.h"
#include "orion-core/exception.h" // orion::OrionException
#include "orion-core/types.h"     // orion::Version
#include "orion-vulkan/config.h"

#ifndef ORION_VULKAN_LOG_LEVEL
    #define ORION_VULKAN_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include <vma/vk_mem_alloc.h> // Vma*
#include <vulkan/vulkan.h>    // Vk*

#include <spdlog/spdlog.h>

namespace orion
{
    // Converts VkResult to human-readable string
    constexpr const char* to_string(VkResult vk_result)
    {
        switch (vk_result) {
            case VK_SUCCESS:
                return "No error occurred.";
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "Host memory allocation failed.";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "Device memory allocation failed.";
            case VK_ERROR_INITIALIZATION_FAILED:
                return "Initialization of an object could not be completed for implementation-specific reasons.";
            case VK_ERROR_LAYER_NOT_PRESENT:
                return "A requested layer is not present or could not be loaded.";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "A requested extension is not supported.";
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
            default:
                break;
        }
        return "Unknown VkResult";
    }

    namespace vulkan
    {
        inline constexpr auto vulkan_api_version = VK_API_VERSION_1_0;

        constexpr std::uint32_t to_vulkan_version(const Version& version) noexcept
        {
            return VK_MAKE_API_VERSION(0, version.major, version.minor, version.patch);
        }

        constexpr Version from_vulkan_version(std::uint32_t version) noexcept
        {
            const auto major = static_cast<std::uint8_t>(VK_API_VERSION_MAJOR(version));
            const auto minor = static_cast<std::uint8_t>(VK_API_VERSION_MINOR(version));
            const auto patch = static_cast<std::uint8_t>(VK_API_VERSION_PATCH(version));
            return Version{major, minor, patch};
        }

        inline void vk_result_check(VkResult vk_result, VkResult expected = VK_SUCCESS)
        {
            if (vk_result != expected) {
                SPDLOG_ERROR("Expected VkResult: {}, got: {}", to_string(expected), to_string(vk_result));
                ORION_DEBUG_BREAK();
            }
        }

        inline const VkAllocationCallbacks* alloc_callbacks() noexcept
        {
            // Create and return a custom allocator here if needed in the future
            return nullptr;
        }
    } // namespace vulkan
} // namespace orion