#pragma once

#include "orion-renderapi/types.h"
#include "vulkan_headers.h"

#include <orion-math/vector/vector2.h> // math::Vector2
#include <string>                      // std::string

namespace orion::vulkan
{
    constexpr auto to_orion_type(VkPhysicalDeviceType physical_device_type) noexcept -> PhysicalDeviceType
    {
        switch (physical_device_type) {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                return PhysicalDeviceType::Integrated;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                return PhysicalDeviceType::Discrete;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                return PhysicalDeviceType::Virtual;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                return PhysicalDeviceType::CPU;
            case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
                break;
        }
        return PhysicalDeviceType::Other;
    }

    constexpr auto to_string(VkQueueFlags queue_flags) -> std::string
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

    constexpr auto to_vulkan_type(Format format) noexcept -> VkFormat
    {
        switch (format) {
            case Format::B8G8R8A8_SRGB:
                return VK_FORMAT_B8G8R8A8_SRGB;
        }
        return VK_FORMAT_UNDEFINED;
    }

    template<typename T>
    constexpr auto to_vulkan_extent(const math::Vector2_t<T>& vec2) noexcept -> VkExtent2D
    {
        return {.width = static_cast<std::uint32_t>(vec2.x()), .height = static_cast<std::uint32_t>(vec2.y())};
    }
} // namespace orion::vulkan
