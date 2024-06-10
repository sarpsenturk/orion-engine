#pragma once

#include "orion-renderapi/buffer.h"
#include "orion-renderapi/image.h"
#include "orion-renderapi/shader.h"

#include "orion-utils/bitflag.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace orion
{
    enum class DescriptorType : std::uint8_t {
        ConstantBuffer,
        StorageBuffer,
        SampledImage,
        Sampler
    };

    ORION_BITFLAG(DescriptorPoolFlags, std::uint8_t){
        FreeDescriptors = 0x1,
    };

    struct DescriptorPoolSize {
        DescriptorType type;
        std::uint32_t count;
    };

    struct DescriptorPoolDesc {
        std::uint32_t max_descriptors;
        DescriptorPoolFlags flags;
        std::span<const DescriptorPoolSize> sizes;
    };

    struct DescriptorBindingDesc {
        DescriptorType type;
        ShaderStageFlags shader_stages;
        std::uint32_t count;

        [[nodiscard]] std::size_t hash() const;
    };
    static_assert(sizeof(DescriptorBindingDesc) == sizeof(std::size_t));

    struct DescriptorLayoutDesc {
        std::span<const DescriptorBindingDesc> bindings;

        [[nodiscard]] std::size_t hash() const;
    };

    struct BufferDescriptorDesc {
        GPUBufferHandle buffer_handle = GPUBufferHandle::invalid();
        BufferRegion region = {};
    };

    struct ImageDescriptorDesc {
        ImageViewHandle image_view_handle = ImageViewHandle::invalid();
        ImageLayout image_layout = ImageLayout::Undefined;
        SamplerHandle sampler_handle = SamplerHandle::invalid();
    };

    // Issues:
    //  Usage of span leads to ugly looking API calls.
    //  However this may be unavoidable.
    struct DescriptorWrite {
        std::uint32_t binding;
        DescriptorType descriptor_type;
        std::uint32_t array_start = 0;
        std::span<const BufferDescriptorDesc> buffers;
        std::span<const ImageDescriptorDesc> images;
    };
} // namespace orion
