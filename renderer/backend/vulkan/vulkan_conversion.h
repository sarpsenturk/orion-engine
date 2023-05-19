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
            case Format::B8G8R8A8_Srgb:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case Format::R32G32_Float:
                return VK_FORMAT_R32G32_SFLOAT;
            case Format::R32G32B32_Float:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case Format::R32G32B32A32_Float:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
        }
        ORION_ASSERT(!"Invalid Format");
        return {};
    }

    constexpr auto to_vulkan_type(ShaderType shader_type) noexcept -> VkShaderStageFlagBits
    {
        switch (shader_type) {
            case ShaderType::Vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderType::Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        ORION_ASSERT(!"Invalid shader type");
        return {};
    }

    constexpr auto to_vulkan_type(InputRate input_rate) noexcept -> VkVertexInputRate
    {
        switch (input_rate) {
            case InputRate::Vertex:
                return VK_VERTEX_INPUT_RATE_VERTEX;
            case InputRate::Instance:
                return VK_VERTEX_INPUT_RATE_INSTANCE;
        }
        ORION_ASSERT(!"Invalid input rate");
        return {};
    }

    constexpr auto to_vulkan_type(PrimitiveTopology topology) noexcept -> VkPrimitiveTopology
    {
        switch (topology) {
            case PrimitiveTopology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
        ORION_ASSERT(!"Invalid primitive topology");
        return {};
    }

    constexpr auto to_vulkan_type(FillMode fill_mode) noexcept -> VkPolygonMode
    {
        switch (fill_mode) {
            case FillMode::Solid:
                return VK_POLYGON_MODE_FILL;
            case FillMode::Wireframe:
                return VK_POLYGON_MODE_LINE;
        }
        ORION_ASSERT(!"Invalid fill mode");
        return {};
    }

    constexpr auto to_vulkan_type(CullMode cull_mode) noexcept -> VkCullModeFlags
    {
        switch (cull_mode) {
            case CullMode::None:
                return VK_CULL_MODE_NONE;
            case CullMode::Front:
                return VK_CULL_MODE_FRONT_BIT;
            case CullMode::Back:
                return VK_CULL_MODE_BACK_BIT;
            case CullMode::FrontAndBack:
                return VK_CULL_MODE_FRONT_AND_BACK;
        }
        ORION_ASSERT(!"Invalid cull mode");
        return {};
    }

    constexpr auto to_vulkan_type(FrontFace front_face) noexcept -> VkFrontFace
    {
        switch (front_face) {
            case FrontFace::CounterClockWise:
                return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            case FrontFace::ClockWise:
                return VK_FRONT_FACE_CLOCKWISE;
        }
        ORION_ASSERT(!"Invalid front face");
        return {};
    }

    constexpr auto to_vulkan_type(GPUBufferUsageFlags buffer_usage) noexcept -> VkBufferUsageFlags
    {
        if (!to_bool(buffer_usage)) {
            return {};
        }
        VkBufferUsageFlags usage_flags = {};
        if (to_bool(buffer_usage & GPUBufferUsageFlags::VertexBuffer)) {
            usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if (to_bool(buffer_usage & GPUBufferUsageFlags::IndexBuffer)) {
            usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if (to_bool(buffer_usage & GPUBufferUsageFlags::TransferSrc)) {
            usage_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }
        if (to_bool(buffer_usage & GPUBufferUsageFlags::TransferDst)) {
            usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }
        return usage_flags;
    }

    constexpr auto to_vulkan_type(const Viewport& viewport) noexcept -> VkViewport
    {
        return {
            .x = viewport.position.x(),
            .y = viewport.position.y(),
            .width = viewport.size.x(),
            .height = viewport.size.y(),
            .minDepth = 0.f,
            .maxDepth = 1.f,
        };
    }

    template<typename T>
    constexpr auto to_vulkan_extent(const math::Vector2_t<T>& vec2) noexcept -> VkExtent2D
    {
        return {.width = static_cast<std::uint32_t>(vec2.x()), .height = static_cast<std::uint32_t>(vec2.y())};
    }
} // namespace orion::vulkan
