#pragma once

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
} // namespace orion
