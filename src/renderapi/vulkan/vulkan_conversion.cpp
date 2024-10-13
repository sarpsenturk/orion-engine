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

    VkImageType to_vk_image_type(ImageType image_type)
    {
        switch (image_type) {
            case ImageType::Image1D:
                return VK_IMAGE_TYPE_1D;
            case ImageType::Image2D:
                return VK_IMAGE_TYPE_2D;
            case ImageType::Image3D:
                return VK_IMAGE_TYPE_3D;
        };
        unreachable();
    }

    VkImageUsageFlags to_vk_image_usage(ImageUsageFlags usage)
    {
        VkImageUsageFlags image_usage = {};
        if ((usage & ImageUsageFlags::RenderTarget) != ImageUsageFlags::None) {
            image_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if ((usage & ImageUsageFlags::DepthStencil) != ImageUsageFlags::None) {
            image_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if ((usage & ImageUsageFlags::ShaderResource) != ImageUsageFlags::None) {
            image_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        return image_usage;
    }

    VkImageViewType to_vk_image_view_type(ImageViewType image_view_type)
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
        unreachable();
    }
} // namespace orion
