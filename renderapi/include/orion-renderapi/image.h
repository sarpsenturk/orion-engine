#pragma once

#include "orion-renderapi/format.h"
#include "orion-renderapi/handles.h"

#include "orion-math/vector/vector3.h"

#include "orion-utils/bitflag.h"

#include <cstdint>

namespace orion
{
    enum class ImageLayout {
        Undefined = 0,
        General,
        ColorAttachment,
        PresentSrc,
        TransferSrc,
        TransferDst,
        ShaderReadOnly
    };

    enum class ImageType {
        Image1D,
        Image2D,
        Image3D
    };

    enum class ImageTiling {
        Optimal,
        Linear
    };

    ORION_BITFLAG(ImageUsageFlags, std::uint8_t){
        TransferSrc = 0x1,
        TransferDst = 0x2,
        ColorAttachment = 0x4,
        DepthStencilAttachment = 0x8,
        InputAttachment = 0x10,
        SampledImage = 0x20,
        Transfer = TransferSrc | TransferDst,
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

    struct ImageDesc {
        ImageType type;
        Format format;
        Vector3_u size;
        ImageTiling tiling;
        ImageUsageFlags usage;
        bool host_visible;
    };

    struct ImageViewDesc {
        ImageHandle image;
        ImageViewType type;
        Format format;
    };

    enum class Filter {
        Nearest,
        Linear
    };

    enum class AddressMode {
        Repeat,
        Mirror,
        Clamp,
        Border
    };

    enum class CompareFunc {
        Never,
        Less,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    struct SamplerDesc {
        Filter filter;
        AddressMode address_mode_u;
        AddressMode address_mode_v;
        AddressMode address_mode_w;
        float mip_lod_bias;
        float max_anisotropy;
        CompareFunc compare_func;
        float min_lod;
        float max_lod;
    };
} // namespace orion
