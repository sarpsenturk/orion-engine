#pragma once

#include <span>

namespace orion
{
    enum class DescriptorType {
        ConstantBuffer,
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
