#pragma once

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

    struct BindGroupLayoutDesc {
        std::span<const DescriptorSetBindingDesc> bindings;
    };

    struct BindGroupDesc {
        BindGroupLayoutHandle layout;
    };

    struct ConstantBufferViewDesc {
        BufferHandle buffer;
        std::uint32_t offset;
        std::uint32_t size;
        BindGroupHandle descriptor_set;
        std::uint32_t descriptor_binding;
    };

    struct ROBufferViewDesc {
        BufferHandle buffer;
        std::uint32_t first_element;
        std::uint32_t element_count;
        std::uint32_t element_size_bytes;
        BindGroupHandle descriptor_set;
        std::uint32_t descriptor_binding;
    };
} // namespace orion
