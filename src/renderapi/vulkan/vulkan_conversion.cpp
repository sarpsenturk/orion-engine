#include "vulkan_conversion.h"

#include "orion/assertion.h"

namespace orion
{
    VkFormat to_vk_format(Format format)
    {
        switch (format) {
            case Format::Undefined:
                return VK_FORMAT_UNDEFINED;
            case Format::B8G8R8A8_Unorm:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case Format::R32G32B32_Float:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case Format::R32G32B32A32_Float:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
        }
        unreachable();
    }

    VkPrimitiveTopology to_vk_primitive_topology(PrimitiveTopology topology)
    {
        switch (topology) {
            case PrimitiveTopology::Point:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case PrimitiveTopology::Line:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case PrimitiveTopology::Triangle:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
        unreachable();
    }

    VkPolygonMode to_vk_polygon_mode(FillMode mode)
    {
        switch (mode) {
            case FillMode::Solid:
                return VK_POLYGON_MODE_FILL;
            case FillMode::Wireframe:
                return VK_POLYGON_MODE_LINE;
        }
        unreachable();
    }

    VkCullModeFlags to_vk_cull_mode(CullMode mode)
    {
        switch (mode) {
            case CullMode::None:
                return VK_CULL_MODE_NONE;
            case CullMode::Back:
                return VK_CULL_MODE_BACK_BIT;
            case CullMode::Front:
                return VK_CULL_MODE_FRONT_BIT;
        }
        unreachable();
    }

    VkBlendFactor to_vk_blend_factor(Blend blend)
    {
        switch (blend) {
            case Blend::Zero:
                return VK_BLEND_FACTOR_ZERO;
            case Blend::One:
                return VK_BLEND_FACTOR_ONE;
        }
        unreachable();
    }

    VkBlendOp to_vk_blend_op(BlendOp blend_op)
    {
        switch (blend_op) {
            case BlendOp::Add:
                return VK_BLEND_OP_ADD;
            case BlendOp::Subtract:
                return VK_BLEND_OP_SUBTRACT;
        }
        unreachable();
    }

    VkColorComponentFlags to_vk_color_components(ColorWriteFlags color_write_mask)
    {
        VkColorComponentFlags color_components = 0;
        if ((color_write_mask & ColorWriteFlags::Red) != ColorWriteFlags::None) {
            color_components |= VK_COLOR_COMPONENT_R_BIT;
        }
        if ((color_write_mask & ColorWriteFlags::Green) != ColorWriteFlags::None) {
            color_components |= VK_COLOR_COMPONENT_G_BIT;
        }
        if ((color_write_mask & ColorWriteFlags::Blue) != ColorWriteFlags::None) {
            color_components |= VK_COLOR_COMPONENT_B_BIT;
        }
        return color_components;
    }

    VkBufferUsageFlags to_vk_buffer_usage(BufferUsage usage)
    {
        switch (usage) {
            case BufferUsage::VertexBuffer:
                return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            case BufferUsage::IndexBuffer:
                return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            case BufferUsage::ConstantBuffer:
                return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        unreachable();
    }

    VkIndexType to_vk_index_type(IndexType index_type)
    {
        switch (index_type) {
            case IndexType::U16:
                return VK_INDEX_TYPE_UINT16;
            case IndexType::U32:
                return VK_INDEX_TYPE_UINT32;
        }
        unreachable();
    }

    VkDescriptorType to_vk_descriptor_type(DescriptorType descriptor_type)
    {
        switch (descriptor_type) {
            case DescriptorType::ConstantBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        unreachable();
    }
} // namespace orion
