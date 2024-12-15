#pragma once

#include "orion/renderapi/format.hpp"
#include "orion/renderapi/handle.hpp"

#include <span>

namespace orion
{
    enum class DescriptorType {
        ConstantBuffer,
        StructuredBuffer,
        ImageView,
        Sampler,
    };

    struct DescriptorSetBindingDesc {
        DescriptorType type;
        std::uint32_t size;
    };

    struct DescriptorSetLayoutDesc {
        std::span<const DescriptorSetBindingDesc> bindings;
    };

    struct DescriptorPoolSize {
        DescriptorType type;
        std::uint32_t count;
    };

    struct DescriptorPoolDesc {
        std::uint32_t max_descriptor_sets;
        std::span<const DescriptorPoolSize> descriptor_sizes;
    };

    struct DescriptorSetDesc {
        DescriptorLayoutHandle layout;
    };

    struct ConstantBufferViewDesc {
        BufferHandle buffer;
        std::uint32_t offset;
        std::uint32_t size;
        DescriptorHandle descriptor_set;
        std::uint32_t descriptor_binding;
    };

    enum class ImageViewType {
        View1D,
        View2D,
        View3D,
        ViewCube,
        View1DArray,
        View2DArray,
        ViewCubeArray,
    };

    struct ImageViewDesc {
        ImageHandle image;
        Format format;
        ImageViewType type;
        DescriptorHandle descriptor_set;
        std::uint32_t descriptor_binding;
    };

    enum class Filter {
        Nearest,
        Linear,
    };

    enum class SamplerAddressMode {
        Wrap,
        Mirror,
        Clamp,
        Border,
    };

    enum class CompareOp {
        None = 0,
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };

    struct SamplerDesc {
        Filter filter;
        SamplerAddressMode address_mode_u;
        SamplerAddressMode address_mode_v;
        SamplerAddressMode address_mode_w;
        float mip_lod_bias;
        CompareOp compare_op;
        float min_lod;
        float max_lod;
        DescriptorHandle descriptor_set;
        std::uint32_t descriptor_binding;
    };

    struct ROBufferViewDesc {
        BufferHandle buffer;
        std::uint32_t first_element;
        std::uint32_t element_count;
        std::uint32_t element_size_bytes;
        DescriptorHandle descriptor_set;
        std::uint32_t descriptor_binding;
    };
} // namespace orion
