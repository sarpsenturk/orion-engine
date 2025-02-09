#include "vulkan_conversion.hpp"

#include "orion/assertion.hpp"

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

    VkBufferUsageFlags to_vk_buffer_usage(BufferUsageFlags usage)
    {
        VkBufferUsageFlags vk_usage_flags = {};
        if ((usage & BufferUsageFlags::VertexBuffer) != BufferUsageFlags::None) {
            vk_usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if ((usage & BufferUsageFlags::IndexBuffer) != BufferUsageFlags::None) {
            vk_usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if ((usage & BufferUsageFlags::ConstantBuffer) != BufferUsageFlags::None) {
            vk_usage_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        if ((usage & BufferUsageFlags::StructuredBuffer) != BufferUsageFlags::None) {
            vk_usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
        if ((usage & BufferUsageFlags::TransferSrc) != BufferUsageFlags::None) {
            vk_usage_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }
        if ((usage & BufferUsageFlags::TransferDst) != BufferUsageFlags::None) {
            vk_usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }
        return vk_usage_flags;
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
            case DescriptorType::StructuredBuffer:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case DescriptorType::ImageView:
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case DescriptorType::Sampler:
                return VK_DESCRIPTOR_TYPE_SAMPLER;
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

    VkFilter to_vk_filter(Filter filter)
    {
        switch (filter) {
            case Filter::Nearest:
                return VK_FILTER_NEAREST;
            case Filter::Linear:
                return VK_FILTER_LINEAR;
        }
        unreachable();
    }

    VkSamplerAddressMode to_vk_address_mode(SamplerAddressMode address_mode)
    {
        switch (address_mode) {
            case SamplerAddressMode::Wrap:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case SamplerAddressMode::Mirror:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case SamplerAddressMode::Clamp:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case SamplerAddressMode::Border:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        unreachable();
    }

    VkCompareOp to_vk_compare_op(CompareOp compare_op)
    {
        switch (compare_op) {
            case CompareOp::None:
                return {};
            case CompareOp::Never:
                return VK_COMPARE_OP_NEVER;
            case CompareOp::Less:
                return VK_COMPARE_OP_LESS;
            case CompareOp::Equal:
                return VK_COMPARE_OP_EQUAL;
            case CompareOp::LessEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareOp::Greater:
                return VK_COMPARE_OP_GREATER;
            case CompareOp::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case CompareOp::GreaterEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareOp::Always:
                return VK_COMPARE_OP_ALWAYS;
        }
        unreachable();
    }

    VkImageLayout to_vk_layout(ImageState image_state)
    {
        switch (image_state) {
            case ImageState::Unknown:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case ImageState::RenderTarget:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ImageState::Present:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            case ImageState::ShaderResource:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        unreachable();
    }
} // namespace orion
