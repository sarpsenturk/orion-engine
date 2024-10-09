#pragma once

#include "orion/renderapi/handle.h"

#include <span>

namespace orion
{
    enum class DescriptorType {
        ConstantBuffer,
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
        DescriptorSetLayoutHandle layout;
        DescriptorPoolHandle pool;
    };

    struct ConstantBufferViewDesc {
        BufferHandle buffer;
        std::uint32_t offset;
        std::uint32_t size;
        DescriptorSetHandle descriptor_set;
        std::uint32_t descriptor_binding;
    };
} // namespace orion
