#pragma once

#include "vulkan_headers.h"

#include "orion-renderapi/types.h"

#include "orion-core/config.h"

#include "orion-math/vector/vector2.h"

#include "orion-utils/assertion.h"

#include <string>

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
        ORION_ASSERT(!"Format not handled in to_vulkan_type() or is invalid");
        return {};
    }

    constexpr auto to_vulkan_type(AttachmentLoadOp load_op) noexcept -> VkAttachmentLoadOp
    {
        switch (load_op) {
            case AttachmentLoadOp::Load:
                return VK_ATTACHMENT_LOAD_OP_LOAD;
            case AttachmentLoadOp::Clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case AttachmentLoadOp::DontCare:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        ORION_ASSERT(!"Load op not handled in to_vulkan_type() or is invalid");
        return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(AttachmentStoreOp store_op) noexcept -> VkAttachmentStoreOp
    {
        switch (store_op) {
            case AttachmentStoreOp::Store:
                return VK_ATTACHMENT_STORE_OP_STORE;
            case AttachmentStoreOp::DontCare:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
        ORION_ASSERT(!"Store op not handled in to_vulkan_type() or is invalid");
        return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(ImageLayout image_layout) noexcept -> VkImageLayout
    {
        switch (image_layout) {
            case ImageLayout::Undefined:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case ImageLayout::ColorAttachment:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ImageLayout::PresentSrc:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            case ImageLayout::TransferSrc:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ImageLayout::TransferDst:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }
        ORION_ASSERT(!"Image layout not handled in to_vulkan_type() or is invalid");
        return VK_IMAGE_LAYOUT_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(ShaderStageFlags shader_stages) noexcept -> VkShaderStageFlags
    {
        if (shader_stages.has_none()) {
            return {};
        }

        VkShaderStageFlags vk_shader_stages = {};
        if (shader_stages.check_and_clear(ShaderStage::Vertex)) {
            vk_shader_stages |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (shader_stages.check_and_clear(ShaderStage::Fragment)) {
            vk_shader_stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        ORION_ASSERT(shader_stages.has_none() &&
                     "Shader stage not handled in to_vulkan_type() or is invalid");
        return vk_shader_stages;
    }

    constexpr auto to_vulkan_type(InputRate input_rate) noexcept -> VkVertexInputRate
    {
        switch (input_rate) {
            case InputRate::Vertex:
                return VK_VERTEX_INPUT_RATE_VERTEX;
            case InputRate::Instance:
                return VK_VERTEX_INPUT_RATE_INSTANCE;
        }
        ORION_ASSERT(!"Input rate not handled in to_vulkan_type() or is invalid");
        return {};
    }

    constexpr auto to_vulkan_type(PrimitiveTopology topology) noexcept -> VkPrimitiveTopology
    {
        switch (topology) {
            case PrimitiveTopology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
        ORION_ASSERT(!"Primitive topology not handled in to_vulkan_type() or is invalid");
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
        ORION_ASSERT(!"Fill mode not handled in to_vulkan_type() or is invalid");
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
        ORION_ASSERT(!"Cull mode not handled in to_vulkan_type() or is invalid");
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
        ORION_ASSERT(!"Front face not handled in to_vulkan_type() or is invalid");
        return {};
    }

    constexpr auto to_vulkan_type(GPUBufferUsageFlags buffer_usage) noexcept -> VkBufferUsageFlags
    {
        if (buffer_usage.has_none()) {
            return {};
        }

        VkBufferUsageFlags usage_flags = {};
        if (buffer_usage.check_and_clear(GPUBufferUsage::VertexBuffer)) {
            usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if (buffer_usage.check_and_clear(GPUBufferUsage::IndexBuffer)) {
            usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if (buffer_usage.check_and_clear(GPUBufferUsage::ConstantBuffer)) {
            usage_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        if (buffer_usage.check_and_clear(GPUBufferUsage::TransferSrc)) {
            usage_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }
        if (buffer_usage.check_and_clear(GPUBufferUsage::TransferDst)) {
            usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }
        ORION_ASSERT(buffer_usage.has_none() &&
                     "Buffer usage flag not handled in to_vulkan_type() or is invalid");
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

    constexpr auto to_vulkan_type(DescriptorType descriptor_type) noexcept -> VkDescriptorType
    {
        switch (descriptor_type) {
            case DescriptorType::ConstantBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            default:
                break;
        }
        ORION_ASSERT(!"Descriptor type not handled in to_vulkan_type() or is invalid");
        return {};
    }

    constexpr auto to_vulkan_type(CommandBufferUsageFlags command_buffer_usage) noexcept -> VkCommandBufferUsageFlags
    {
        if (command_buffer_usage.has_none()) {
            return {};
        }

        VkCommandBufferUsageFlags usage_flags = {};
        if (command_buffer_usage.check_and_clear(CommandBufferUsage::OneTimeSubmit)) {
            usage_flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        ORION_ASSERT(command_buffer_usage.has_none() &&
                     "Command buffer usage flag not handled in to_vulkan_type() or is invalid");
        return {};
    }

    constexpr auto to_vulkan_type(ImageType image_type) noexcept -> VkImageType
    {
        switch (image_type) {
            case ImageType::Image1D:
                return VK_IMAGE_TYPE_1D;
            case ImageType::Image2D:
                return VK_IMAGE_TYPE_2D;
            case ImageType::Image3D:
                return VK_IMAGE_TYPE_3D;
        }
        ORION_ASSERT(!"ImageType not handled in to_vulkan_type()");
        return {};
    }

    constexpr auto to_vulkan_type(ImageTiling image_tiling) noexcept -> VkImageTiling
    {
        switch (image_tiling) {
            case ImageTiling::Optimal:
                return VK_IMAGE_TILING_OPTIMAL;
            case ImageTiling::Linear:
                return VK_IMAGE_TILING_LINEAR;
        }
        ORION_ASSERT(!"ImageTiling not handled in to_vulkan_type()");
        return {};
    }

    constexpr auto to_vulkan_type(ImageUsageFlags image_usage_flags) noexcept -> VkImageUsageFlags
    {
        if (image_usage_flags.has_none()) {
            return {};
        }

        VkImageUsageFlags vk_usage_flags = {};
        if (image_usage_flags.check_and_clear(ImageUsage::TransferSrc)) {
            vk_usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        if (image_usage_flags.check_and_clear(ImageUsage::TransferDst)) {
            vk_usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        if (image_usage_flags.check_and_clear(ImageUsage::ColorAttachment)) {
            vk_usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if (image_usage_flags.check_and_clear(ImageUsage::DepthStencilAttachment)) {
            vk_usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (image_usage_flags.check_and_clear(ImageUsage::InputAttachment)) {
            vk_usage_flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
        ORION_ASSERT(image_usage_flags.has_none() &&
                     "Image usage not handled in to_vulkan_type()");
        return vk_usage_flags;
    }

    constexpr auto to_vulkan_type(ImageViewType image_view_type) noexcept -> VkImageViewType
    {
        switch (image_view_type) {
            case ImageViewType::View1D:
                return VK_IMAGE_VIEW_TYPE_1D;
            case ImageViewType::View2D:
                return VK_IMAGE_VIEW_TYPE_2D;
            case ImageViewType::View3D:
                return VK_IMAGE_VIEW_TYPE_3D;
            case ImageViewType::ViewCube:
                return VK_IMAGE_VIEW_TYPE_CUBE;
            case ImageViewType::View1DArray:
                return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            case ImageViewType::View2DArray:
                return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            case ImageViewType::ViewCubeArray:
                return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        }
        ORION_ASSERT(!"ImageViewType not handled in to_vulkan_type()");
        return {};
    }

    template<typename T>
    constexpr auto to_vulkan_extent(const Vector2_t<T>& vec2) noexcept -> VkExtent2D
    {
        return {
            .width = static_cast<std::uint32_t>(vec2.x()),
            .height = static_cast<std::uint32_t>(vec2.y()),
        };
    }

    template<typename T>
    constexpr auto to_vulkan_extent(const Vector3_t<T>& vec3) noexcept -> VkExtent3D
    {
        return {
            .width = static_cast<std::uint32_t>(vec3.x()),
            .height = static_cast<std::uint32_t>(vec3.y()),
            .depth = static_cast<std::uint32_t>(vec3.z()),
        };
    }
} // namespace orion::vulkan
