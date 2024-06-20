#include "vulkan_conversion.h"

#include "orion-utils/assertion.h"

#include <unordered_map>

namespace orion::vulkan
{
    namespace
    {
        template<typename From, typename To>
        VkFlags convert_with_map(From value, const std::unordered_map<From, To>& map, [[maybe_unused]] const char* message)
        {
            VkFlags result{};
            while (!!value) {
                for (auto [from, to] : map) {
                    if (!!(value & from)) {
                        result |= to;
                        value ^= from;
                        break;
                    }
                }
                ORION_ASSERT(message);
            }
            return result;
        }
    } // namespace

    PhysicalDeviceType to_orion_type(VkPhysicalDeviceType physical_device_type)
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

    VkFormat to_vulkan_format(Format format)
    {
        switch (format) {
            case Format::R8_Unorm:
                return VK_FORMAT_R8_UNORM;
            case Format::B8G8R8A8_Srgb:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case Format::R32_Uint:
                return VK_FORMAT_R32_UINT;
            case Format::R32_Int:
                return VK_FORMAT_R32_SINT;
            case Format::R32_Float:
                return VK_FORMAT_R32_SFLOAT;
            case Format::R32G32_Uint:
                return VK_FORMAT_R32G32_UINT;
            case Format::R32G32_Int:
                return VK_FORMAT_R32G32_SINT;
            case Format::R32G32_Float:
                return VK_FORMAT_R32G32_SFLOAT;
            case Format::R32G32B32_Uint:
                return VK_FORMAT_R32G32B32_UINT;
            case Format::R32G32B32_Int:
                return VK_FORMAT_R32G32B32_SINT;
            case Format::R32G32B32_Float:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case Format::R32G32B32A32_Uint:
                return VK_FORMAT_R32G32B32A32_UINT;
            case Format::R32G32B32A32_Int:
                return VK_FORMAT_R32G32B32A32_SINT;
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

    VkImageLayout to_vulkan_type(ImageLayout image_layout)
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

    VkShaderStageFlags to_vulkan_type(ShaderStageFlags shader_stages)
    {
        static const auto conversion_map = std::unordered_map{
            std::make_pair(ShaderStageFlags::Vertex, VK_SHADER_STAGE_VERTEX_BIT),
            std::make_pair(ShaderStageFlags::Pixel, VK_SHADER_STAGE_FRAGMENT_BIT),
        };
        return convert_with_map(shader_stages, conversion_map, "Shader stage not handled in to_vulkan_type()");
    }

    VkPrimitiveTopology to_vulkan_type(PrimitiveTopology topology)
    {
        switch (topology) {
            case PrimitiveTopology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
        ORION_ASSERT(!"Primitive topology not handled in to_vulkan_type() or is invalid");
        return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    }

    VkPolygonMode to_vulkan_type(FillMode fill_mode)
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

    VkCullModeFlags to_vulkan_type(CullMode cull_mode)
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

    VkFrontFace to_vulkan_type(FrontFace front_face)
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

    VkBufferUsageFlags to_vulkan_type(GPUBufferUsageFlags buffer_usage)
    {
        const static auto conversion_map = std::unordered_map{
            std::make_pair(GPUBufferUsageFlags::VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
            std::make_pair(GPUBufferUsageFlags::IndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
            std::make_pair(GPUBufferUsageFlags::ConstantBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
            std::make_pair(GPUBufferUsageFlags::TransferSrc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT),
            std::make_pair(GPUBufferUsageFlags::TransferDst, VK_BUFFER_USAGE_TRANSFER_DST_BIT),
            std::make_pair(GPUBufferUsageFlags::StorageBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT),
        };

        return convert_with_map(buffer_usage, conversion_map, "Buffer usage flag not handled in to_vulkan_type()");
    }

    VkViewport to_vulkan_type(const Viewport& viewport)
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

    VkDescriptorPoolCreateFlags to_vulkan_type(DescriptorPoolFlags flags)
    {
        const static auto conversion_map = std::unordered_map{
            std::make_pair(DescriptorPoolFlags::FreeDescriptors, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT),
        };
        return convert_with_map(flags, conversion_map, "Descriptor pool flag not handled in to_vulkan_type()");
    }

    VkDescriptorType to_vulkan_type(DescriptorType descriptor_type)
    {
        switch (descriptor_type) {
            case DescriptorType::ConstantBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case DescriptorType::StorageBuffer:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case DescriptorType::SampledImage:
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case DescriptorType::Sampler:
                return VK_DESCRIPTOR_TYPE_SAMPLER;
        }
        ORION_ASSERT(!"Descriptor type not handled in to_vulkan_type() or is invalid");
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    VkImageType to_vulkan_type(ImageType image_type)
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

    VkImageTiling to_vulkan_type(ImageTiling image_tiling)
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

    VkImageUsageFlags to_vulkan_type(ImageUsageFlags image_usage_flags)
    {
        static const auto conversion_map = std::unordered_map{
            std::make_pair(ImageUsageFlags::TransferSrc, VK_IMAGE_USAGE_TRANSFER_SRC_BIT),
            std::make_pair(ImageUsageFlags::TransferDst, VK_IMAGE_USAGE_TRANSFER_DST_BIT),
            std::make_pair(ImageUsageFlags::ColorAttachment, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
            std::make_pair(ImageUsageFlags::InputAttachment, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
            std::make_pair(ImageUsageFlags::SampledImage, VK_IMAGE_USAGE_SAMPLED_BIT),
            std::make_pair(ImageUsageFlags::SampledImage, VK_IMAGE_USAGE_SAMPLED_BIT),
        };
        return convert_with_map(image_usage_flags, conversion_map, "Image usage not handled in to_vulkan_type()");
    }

    VkImageViewType to_vulkan_type(ImageViewType image_view_type)
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

    VkIndexType to_vulkan_type(IndexType index_type)
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

    VkFilter to_vulkan_type(Filter filter)
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

    VkSamplerAddressMode to_vulkan_type(AddressMode address_mode)
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

    VkCompareOp to_vulkan_type(CompareFunc compare_func)
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

    VkBlendFactor to_vulkan_type(BlendFactor blend_factor)
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

    VkBlendOp to_vulkan_type(BlendOp blend_op)
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

    VkColorComponentFlags to_vulkan_type(ColorComponentFlags color_component_flags)
    {
        static const auto conversion_map = std::unordered_map{
            std::make_pair(ColorComponentFlags::R, VK_COLOR_COMPONENT_R_BIT),
            std::make_pair(ColorComponentFlags::G, VK_COLOR_COMPONENT_G_BIT),
            std::make_pair(ColorComponentFlags::B, VK_COLOR_COMPONENT_B_BIT),
            std::make_pair(ColorComponentFlags::A, VK_COLOR_COMPONENT_A_BIT),
        };
        return convert_with_map(color_component_flags, conversion_map, "Color component not handled in to_vulkan_type()");
    }

    VkLogicOp to_vulkan_type(LogicOp logic_op)
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

    VkPipelineBindPoint to_vulkan_type(PipelineBindPoint bind_point)
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

    VkViewport to_vulkan_viewport(const Viewport& viewport)
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

    VkRect2D to_vulkan_scissor(const Scissor& scissor)
    {
        return {
            .offset = {scissor.offset.x(), scissor.offset.y()},
            .extent = {scissor.size.x(), scissor.size.y()},
        };
    }

    VkExtent2D to_vulkan_extent(const Vector2_u& vec2)
    {
        return {
            .width = vec2.x(),
            .height = vec2.y(),
        };
    }

    VkExtent3D to_vulkan_extent(const Vector3_u& vec3)
    {
        return {
            .width = vec3.x(),
            .height = vec3.y(),
            .depth = vec3.z(),
        };
    }

    VkOffset2D to_vulkan_offset(const Vector2_i& vec2)
    {
        return {
            .x = vec2.x(),
            .y = vec2.y(),
        };
    }

    VkRect2D to_vulkan_rect(const Rect2D& rect2d)
    {
        return {
            .offset = to_vulkan_offset(rect2d.offset),
            .extent = to_vulkan_extent(rect2d.size),
        };
    }

    VkClearColorValue to_vulkan_clear_color(const Vector4_f& color)
    {
        return {color[0], color[1], color[2], color[3]};
    }

    VkAttachmentDescription to_vulkan_attachment(Format format)
    {
        return {
            .flags = 0,
            .format = to_vulkan_format(format),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
    }
} // namespace orion::vulkan
