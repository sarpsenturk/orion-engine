#pragma once

#include "orion/renderer/gpu/gpu.hpp"

#include <cstdint>

namespace orion
{
    template<typename Tag>
    struct RGResource {
        static constexpr auto invalid_index = ~0u;

        std::uint32_t index   = invalid_index;
        std::uint32_t version = 0u;
    };

    using RGTexture = RGResource<struct RGTextureTag>;
    using RGBuffer  = RGResource<struct RGBufferTag>;

    struct RGResourceState {
        gpu::PipelineStage stages = gpu::PipelineStage::None;
        gpu::Access access        = gpu::Access::None;
        gpu::ImageLayout layout   = gpu::ImageLayout::Undefined;
    };

    struct RGTextureDesc {
        std::uint32_t width  = 0u;
        std::uint32_t height = 0u;
        std::uint32_t depth  = 0u;
        gpu::Format format   = gpu::Format::Undefined;
    };

    struct RGTextureImportDesc {
        gpu::Image* image             = nullptr;
        gpu::ImageView* image_view    = nullptr;
        RGResourceState initial_state = {};
        RGResourceState final_state   = {};
    };

    struct RGBufferDesc {
        std::uint64_t size = 0u;
    };

    struct RGBufferImportDesc {
        gpu::Buffer* buffer           = nullptr;
        RGResourceState initial_state = {};
        RGResourceState final_state   = {};
    };
} // namespace orion
