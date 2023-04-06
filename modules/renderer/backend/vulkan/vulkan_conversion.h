#pragma once

#include "orion-renderapi/types.h"
#include "vulkan_headers.h"

#include <orion-math/vector/vector2.h> // math::Vector2
#include <orion-utils/assertion.h>     // ORION_ASSERT
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
        ORION_ASSERT(!"Invalid Format");
        return {};
    }

    constexpr auto to_vulkan_type(AttachmentLoadOp load_op) -> VkAttachmentLoadOp
    {
        switch (load_op) {
            case AttachmentLoadOp::Load:
                return VK_ATTACHMENT_LOAD_OP_LOAD;
            case AttachmentLoadOp::Clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case AttachmentLoadOp::DontCare:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        ORION_ASSERT(!"Invalid AttachmentLoadOp");
        return {};
    }

    constexpr auto to_vulkan_type(AttachmentStoreOp store_op) -> VkAttachmentStoreOp
    {
        switch (store_op) {
            case AttachmentStoreOp::Store:
                return VK_ATTACHMENT_STORE_OP_STORE;
            case AttachmentStoreOp::DontCare:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
        ORION_ASSERT(!"Invalid AttachmentStoreOp");
        return {};
    }

    constexpr auto to_vulkan_type(ImageLayout image_layout) -> VkImageLayout
    {
        switch (image_layout) {
            case ImageLayout::Undefined:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case ImageLayout::ColorAttachment:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ImageLayout::PresentSrc:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        ORION_ASSERT(!"Invalid ImageLayout");
        return {};
    }

    template<typename T>
    constexpr auto to_vulkan_extent(const math::Vector2_t<T>& vec2) noexcept -> VkExtent2D
    {
        return {.width = static_cast<std::uint32_t>(vec2.x()), .height = static_cast<std::uint32_t>(vec2.y())};
    }
} // namespace orion::vulkan
