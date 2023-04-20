#pragma once

#include "orion-core/exception.h" // orion::OrionException
#include "orion-core/types.h"     // orion::Version
#include "orion-vulkan/config.h"

#include <spdlog/logger.h>    // spdlog::logger
#include <vma/vk_mem_alloc.h> // Vma*
#include <vulkan/vulkan.h>    // Vk*

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

    class VulkanException : public OrionException
    {
    public:
        explicit VulkanException(VkResult vk_result)
            : vk_result_(vk_result)
            , result_string_(to_string(vk_result))
        {
        }

        [[nodiscard]] const char* type() const noexcept override
        {
            return "VulkanError";
        }
        [[nodiscard]] int return_code() const noexcept override
        {
            return vk_result_;
        }
        [[nodiscard]] const char* what() const override
        {
            return result_string_.c_str();
        }

        [[nodiscard]] auto result() const noexcept
        {
            return vk_result_;
        }

    private:
        VkResult vk_result_;
        std::string result_string_;
    };

    namespace vulkan
    {
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
                throw VulkanException(vk_result);
            }
        }

        inline const VkAllocationCallbacks* alloc_callbacks() noexcept
        {
            // Create and return a custom allocator here if needed in the future
            return nullptr;
        }

        std::shared_ptr<spdlog::logger> logger();

        spdlog::logger* logger_raw();
    } // namespace vulkan
} // namespace orion
