#pragma once

#include "orion-core/defs.h"
#include "orion-core/exception.h"
#include "orion-core/version.h"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include <initializer_list>
#include <string>

constexpr auto format_as(VkResult vk_result)
{
    switch (vk_result) {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_FRAGMENTED_POOL:
            return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return "VK_ERROR_OUT_OF_POOL_MEMORY";
        default:
            break;
    }
    return "Unknown VkResult";
}

constexpr auto format_as(VkQueueFlagBits queue_flags) -> std::string
{
    if (!queue_flags) {
        return {};
    }

    std::string result;
    if (queue_flags & VK_QUEUE_GRAPHICS_BIT) {
        result += "Graphics | ";
    }
    if (queue_flags & VK_QUEUE_COMPUTE_BIT) {
        result += "Compute | ";
    }
    if (queue_flags & VK_QUEUE_TRANSFER_BIT) {
        result += "Transfer | ";
    }
    return result.substr(0, result.size() - 3);
}

namespace orion
{
    // Gets VkResult description
    constexpr const char* vk_result_desc(VkResult vk_result)
    {
        switch (vk_result) {
            case VK_SUCCESS:
                return "No error occurred.";
            case VK_NOT_READY:
                return "A fence or query has not yet completed";
            case VK_TIMEOUT:
                return "A wait operation has not completed in the specified time";
            case VK_SUBOPTIMAL_KHR:
                return "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully";
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
            case VK_ERROR_FRAGMENTED_POOL:
                return "A pool allocation has failed due to fragmentation of the pools memory.";
            case VK_ERROR_OUT_OF_POOL_MEMORY:
                return "A pool memory allocation has failed";
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
            static constexpr auto error_string = "\n--- VkResult Check Failed ---\n"
                                                 "Expected VkResult: {}, got: {}\n"
                                                 "-----------------------------";
            if (vk_result != expected) {
                SPDLOG_ERROR(error_string, expected, vk_result);
                ORION_DEBUG_BREAK();
            }
        }

        inline void vk_result_check(VkResult vk_result, std::initializer_list<VkResult> expected_results)
        {
            static constexpr auto error_string = "\n--- VkResult Check Failed ---\n"
                                                 "Expected VkResult to match: {}, got: {}\n"
                                                 "-----------------------------";
            for (auto result : expected_results) {
                if (vk_result == result) {
                    return;
                }
            }
            SPDLOG_ERROR(error_string, expected_results, vk_result);
            ORION_DEBUG_BREAK();
        }

        inline const VkAllocationCallbacks* alloc_callbacks() noexcept
        {
            // Create and return a custom allocator here if needed in the future
            return nullptr;
        }
    } // namespace vulkan
} // namespace orion
