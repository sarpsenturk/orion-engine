#pragma once

#include "orion/renderapi/format.h"

#include <cstdint>
#include <type_traits>

namespace orion
{
    enum class ImageType {
        Image1D,
        Image2D,
        Image3D,
    };

    enum class ImageUsageFlags : std::uint8_t {
        None = 0,
        RenderTarget = 1,
        DepthStencil = 2,
        ShaderResource = 4,
    };

    // TODO: Allow declaring bit flags automatically
    constexpr ImageUsageFlags operator&(ImageUsageFlags lhs, ImageUsageFlags rhs)
    {
        return static_cast<ImageUsageFlags>(static_cast<std::underlying_type_t<ImageUsageFlags>>(lhs) & static_cast<std::underlying_type_t<ImageUsageFlags>>(rhs));
    }

    struct ImageDesc {
        std::uint32_t width;
        std::uint32_t height;
        std::uint32_t depth;
        Format format;
        ImageType type;
        ImageUsageFlags usage_flags;
    };
} // namespace orion
