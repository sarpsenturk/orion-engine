#pragma once

#include "orion/renderapi/format.hpp"
#include "orion/renderapi/handle.hpp"

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
    constexpr ImageUsageFlags operator|(ImageUsageFlags lhs, ImageUsageFlags rhs)
    {
        return static_cast<ImageUsageFlags>(static_cast<std::underlying_type_t<ImageUsageFlags>>(lhs) | static_cast<std::underlying_type_t<ImageUsageFlags>>(rhs));
    }

    struct ImageDesc {
        std::uint32_t width;
        std::uint32_t height;
        std::uint32_t depth;
        Format format;
        ImageType type;
        ImageUsageFlags usage_flags;
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
        ImageViewType type;
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
    };
} // namespace orion
