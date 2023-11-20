#pragma once

#include "vulkan_headers.h"

#include "orion-renderapi/defs.h"

#include "orion-math/vector/vector2.h"

#include "orion-utils/assertion.h"

#include <numeric>
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
            case Format::R8_Unorm:
                return VK_FORMAT_R8_UNORM;
            case Format::B8G8R8A8_Srgb:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case Format::R32G32_Float:
                return VK_FORMAT_R32G32_SFLOAT;
            case Format::R32G32B32_Float:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case Format::R32G32B32A32_Float:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case Format::R8G8B8A8_Unorm:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case Format::Undefined:
                break;
        }
        ORION_ASSERT(!"Format not handled in to_vulkan_type() or is invalid");
        return VK_FORMAT_MAX_ENUM;
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
            case ImageLayout::General:
                return VK_IMAGE_LAYOUT_GENERAL;
            case ImageLayout::ColorAttachment:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ImageLayout::PresentSrc:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            case ImageLayout::TransferSrc:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ImageLayout::TransferDst:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case ImageLayout::ShaderReadOnly:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        ORION_ASSERT(!"Image layout not handled in to_vulkan_type() or is invalid");
        return VK_IMAGE_LAYOUT_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(ShaderStageFlags shader_stages) noexcept -> VkShaderStageFlags
    {
        auto conversion_fn = [](auto acc, ShaderStageFlags shader_stage) -> VkShaderStageFlags {
            if (!shader_stage) {
                return acc;
            }
            switch (shader_stage) {
                case ShaderStageFlags::Vertex:
                    return acc | VK_SHADER_STAGE_VERTEX_BIT;
                case ShaderStageFlags::Pixel:
                    return acc | VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            ORION_ASSERT(!"Shader stage not handled in to_vulkan_type()");
            return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        };
        const auto bitwise_range = BitwiseRange{shader_stages};
        return std::accumulate(bitwise_range.begin(), bitwise_range.end(), VkShaderStageFlags{}, conversion_fn);
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
        return VK_VERTEX_INPUT_RATE_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(PrimitiveTopology topology) noexcept -> VkPrimitiveTopology
    {
        switch (topology) {
            case PrimitiveTopology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
        ORION_ASSERT(!"Primitive topology not handled in to_vulkan_type() or is invalid");
        return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
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
        return VK_POLYGON_MODE_MAX_ENUM;
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
        return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
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
        return VK_FRONT_FACE_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(GPUBufferUsageFlags buffer_usage) noexcept -> VkBufferUsageFlags
    {
        auto conversion_fn = [](auto acc, GPUBufferUsageFlags buffer_usage) -> VkBufferUsageFlags {
            if (!buffer_usage) {
                return acc;
            }
            switch (buffer_usage) {
                case GPUBufferUsageFlags::VertexBuffer:
                    return acc | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                case GPUBufferUsageFlags::IndexBuffer:
                    return acc | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                case GPUBufferUsageFlags::ConstantBuffer:
                    return acc | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                case GPUBufferUsageFlags::TransferSrc:
                    return acc | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                case GPUBufferUsageFlags::TransferDst:
                    return acc | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            ORION_ASSERT("Buffer usage flag not handled in to_vulkan_type()");
            return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        };
        const auto bitwise_range = BitwiseRange{buffer_usage};
        return std::accumulate(bitwise_range.begin(), bitwise_range.end(), VkBufferUsageFlags{}, conversion_fn);
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

    constexpr auto to_vulkan_type(BindingType descriptor_type) noexcept -> VkDescriptorType
    {
        switch (descriptor_type) {
            case BindingType::ConstantBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        ORION_ASSERT(!"Descriptor type not handled in to_vulkan_type() or is invalid");
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
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
        return VK_IMAGE_TYPE_MAX_ENUM;
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
        return VK_IMAGE_TILING_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(ImageUsageFlags image_usage_flags) noexcept -> VkImageUsageFlags
    {
        auto conversion_fn = [](auto acc, ImageUsageFlags usage_flags) -> VkImageUsageFlags {
            if (!usage_flags) {
                return acc;
            }
            switch (usage_flags) {
                case ImageUsageFlags::TransferSrc:
                    return acc | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                case ImageUsageFlags::TransferDst:
                    return acc | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                case ImageUsageFlags::ColorAttachment:
                    return acc | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                case ImageUsageFlags::DepthStencilAttachment:
                    return acc | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                case ImageUsageFlags::InputAttachment:
                    return acc | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
                case ImageUsageFlags::SampledImage:
                    return acc | VK_IMAGE_USAGE_SAMPLED_BIT;
            }
            ORION_ASSERT("Image usage not handled in to_vulkan_type()");
            return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        };
        const auto bitwise_range = BitwiseRange{image_usage_flags};
        return std::accumulate(bitwise_range.begin(), bitwise_range.end(), VkImageUsageFlags{}, conversion_fn);
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
        return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(IndexType index_type) -> VkIndexType
    {
        switch (index_type) {
            case IndexType::Uint16:
                return VK_INDEX_TYPE_UINT16;
            case IndexType::Uint32:
                return VK_INDEX_TYPE_UINT32;
        }
        ORION_ASSERT(!"Index type not handled in to_vulkan_type()");
        return VK_INDEX_TYPE_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(Filter filter) -> VkFilter
    {
        switch (filter) {
            case Filter::Nearest:
                return VK_FILTER_NEAREST;
            case Filter::Linear:
                return VK_FILTER_LINEAR;
        }
        ORION_ASSERT(!"Filter type not handled in to_vulkan_type()");
        return VK_FILTER_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(AddressMode address_mode) -> VkSamplerAddressMode
    {
        switch (address_mode) {
            case AddressMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case AddressMode::Mirror:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case AddressMode::Clamp:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case AddressMode::Border:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        ORION_ASSERT(!"Address mode not handled in to_vulkan_type()");
        return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(CompareFunc compare_func) -> VkCompareOp
    {
        switch (compare_func) {
            case CompareFunc::Never:
                return VK_COMPARE_OP_NEVER;
            case CompareFunc::Less:
                return VK_COMPARE_OP_LESS;
            case CompareFunc::LessOrEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareFunc::Greater:
                return VK_COMPARE_OP_GREATER;
            case CompareFunc::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case CompareFunc::GreaterOrEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareFunc::Always:
                return VK_COMPARE_OP_ALWAYS;
        }
        ORION_ASSERT(!"Compare function not handled in to_vulkan_type()");
        return VK_COMPARE_OP_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(BlendFactor blend_factor) -> VkBlendFactor
    {
        switch (blend_factor) {
            case BlendFactor::Zero:
                return VK_BLEND_FACTOR_ZERO;
            case BlendFactor::One:
                return VK_BLEND_FACTOR_ONE;
            case BlendFactor::SrcColor:
                return VK_BLEND_FACTOR_SRC_COLOR;
            case BlendFactor::InvertedSrcColor:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case BlendFactor::DstColor:
                return VK_BLEND_FACTOR_DST_COLOR;
            case BlendFactor::InvertedDstColor:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case BlendFactor::SrcAlpha:
                return VK_BLEND_FACTOR_SRC_ALPHA;
            case BlendFactor::InvertedSrcAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstAlpha:
                return VK_BLEND_FACTOR_DST_ALPHA;
            case BlendFactor::InvertedDstAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        }
        ORION_ASSERT(!"Blend factor not handled in to_vulkan_type()");
        return VK_BLEND_FACTOR_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(BlendOp blend_op) -> VkBlendOp
    {
        switch (blend_op) {
            case BlendOp::Add:
                return VK_BLEND_OP_ADD;
            case BlendOp::Subtract:
                return VK_BLEND_OP_SUBTRACT;
            case BlendOp::ReverseSubtract:
                return VK_BLEND_OP_REVERSE_SUBTRACT;
            case BlendOp::Min:
                return VK_BLEND_OP_MIN;
            case BlendOp::Max:
                return VK_BLEND_OP_MAX;
        }
        ORION_ASSERT(!"Blend factor not handled in to_vulkan_type()");
        return VK_BLEND_OP_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(ColorComponentFlags color_component_flags) -> VkColorComponentFlags
    {
        auto conversion_fn = [](auto acc, ColorComponentFlags component_flags) -> VkColorComponentFlags {
            if (!component_flags) {
                return acc;
            }
            switch (component_flags) {
                case ColorComponentFlags::R:
                    return acc | VK_COLOR_COMPONENT_R_BIT;
                case ColorComponentFlags::G:
                    return acc | VK_COLOR_COMPONENT_G_BIT;
                case ColorComponentFlags::B:
                    return acc | VK_COLOR_COMPONENT_B_BIT;
                case ColorComponentFlags::A:
                    return acc | VK_COLOR_COMPONENT_A_BIT;
            }
            ORION_ASSERT("Color component not handled in to_vulkan_type()");
            return VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM;
        };
        const auto bitwise_range = BitwiseRange{color_component_flags};
        return std::accumulate(bitwise_range.begin(), bitwise_range.end(), VkColorComponentFlags{}, conversion_fn);
    }

    constexpr auto to_vulkan_type(LogicOp logic_op) -> VkLogicOp
    {
        switch (logic_op) {
            case LogicOp::NoOp:
                return VK_LOGIC_OP_NO_OP;
            case LogicOp::Clear:
                return VK_LOGIC_OP_CLEAR;
            case LogicOp::And:
                return VK_LOGIC_OP_AND;
            case LogicOp::AndReverse:
                return VK_LOGIC_OP_AND_REVERSE;
            case LogicOp::AndInverted:
                return VK_LOGIC_OP_AND_INVERTED;
            case LogicOp::Nand:
                return VK_LOGIC_OP_NAND;
            case LogicOp::Or:
                return VK_LOGIC_OP_OR;
            case LogicOp::OrReverse:
                return VK_LOGIC_OP_OR_REVERSE;
            case LogicOp::OrInverted:
                return VK_LOGIC_OP_OR_INVERTED;
            case LogicOp::Copy:
                return VK_LOGIC_OP_COPY;
            case LogicOp::CopyInverted:
                return VK_LOGIC_OP_COPY_INVERTED;
            case LogicOp::Xor:
                return VK_LOGIC_OP_XOR;
            case LogicOp::Nor:
                return VK_LOGIC_OP_NOR;
            case LogicOp::Equivalent:
                return VK_LOGIC_OP_EQUIVALENT;
            case LogicOp::Set:
                return VK_LOGIC_OP_SET;
        }
        ORION_ASSERT(!"Logic op not handled in to_vulkan_type()");
        return VK_LOGIC_OP_MAX_ENUM;
    }

    constexpr auto to_vulkan_type(PipelineBindPoint bind_point) -> VkPipelineBindPoint
    {
        switch (bind_point) {
            case PipelineBindPoint::Graphics:
                return VK_PIPELINE_BIND_POINT_GRAPHICS;
            case PipelineBindPoint::Compute:
                return VK_PIPELINE_BIND_POINT_COMPUTE;
        }
        ORION_ASSERT(!"Pipeline bind point not handled in to_vulkan_type()");
        return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }

    constexpr auto to_vulkan_viewport(const Viewport& viewport) -> VkViewport
    {
        return {
            .x = viewport.position.x(),
            .y = viewport.position.y(),
            .width = viewport.size.x(),
            .height = viewport.size.y(),
            .minDepth = viewport.depth.x(),
            .maxDepth = viewport.depth.y(),
        };
    }

    constexpr auto to_vulkan_scissor(const Scissor& scissor) -> VkRect2D
    {
        return {
            .offset = {scissor.offset.x(), scissor.offset.y()},
            .extent = {scissor.size.x(), scissor.size.y()},
        };
    }

    constexpr auto to_vulkan_extent(const Vector2_u& vec2) noexcept -> VkExtent2D
    {
        return {
            .width = vec2.x(),
            .height = vec2.y(),
        };
    }

    constexpr auto to_vulkan_extent(const Vector3_u& vec3) noexcept -> VkExtent3D
    {
        return {
            .width = vec3.x(),
            .height = vec3.y(),
            .depth = vec3.z(),
        };
    }

    constexpr auto to_vulkan_offset(const Vector2_i& vec2) noexcept -> VkOffset2D
    {
        return {
            .x = vec2.x(),
            .y = vec2.y(),
        };
    }

    constexpr auto to_vulkan_rect(const Rect2D& rect2d) -> VkRect2D
    {
        return {
            .offset = to_vulkan_offset(rect2d.offset),
            .extent = to_vulkan_extent(rect2d.size),
        };
    }

    constexpr auto to_vulkan_clear_color(const Vector4_f& color) -> VkClearColorValue
    {
        return {color[0], color[1], color[2], color[3]};
    }
} // namespace orion::vulkan
