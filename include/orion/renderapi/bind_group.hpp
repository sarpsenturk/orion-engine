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

    struct BufferBindingDesc {
        std::uint32_t binding;
        BufferHandle buffer;
        DescriptorType type;
        std::uint32_t offset;
        std::uint32_t size;
    };

    struct ImageViewBindingDesc {
        std::uint32_t binding;
        ImageViewHandle image_view;
    };

    struct SamplerBindingDesc {
        std::uint32_t binding;
        SamplerHandle sampler;
    };

    struct BindGroupDesc {
        BindGroupLayoutHandle layout;
        std::span<const BufferBindingDesc> buffers;
        std::span<const ImageViewBindingDesc> views;
        std::span<const SamplerBindingDesc> samplers;
    };
} // namespace orion
