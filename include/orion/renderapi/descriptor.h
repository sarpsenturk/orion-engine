#pragma once

#include "orion/renderapi/format.h"
#include "orion/renderapi/handle.h"

#include <span>

namespace orion
{
    enum class DescriptorType {
        ConstantBuffer,
        ImageView,
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
        DescriptorSetHandle descriptor_set;
        std::uint32_t descriptor_binding;
    };
} // namespace orion
