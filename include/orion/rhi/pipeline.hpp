#pragma once

#include "orion/rhi/format.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace orion
{
    // Pipeline vertex input

    struct RHIVertexAttribute {
        const char* name;
        RHIFormat format;
    };

    struct RHIVertexBinding {
        std::span<const RHIVertexAttribute> attributes;
    };

    // Pipeline input assembly

    enum class RHIPrimitiveTopology {
        TriangleList,
    };

    struct RHIInputAssemblyDesc {
        RHIPrimitiveTopology topology;
    };

    // Pipeline rasterization

    enum class RHIFrontFace {
        Clockwise,
        CounterClockwise,
    };

    enum class RHIFillMode {
        Solid,
    };

    enum class RHICullMode {
        None,
        Front,
        Back,
    };

    struct RHIRasterizerDesc {
        RHIFillMode fill_mode;
        RHICullMode cull_mode;
        RHIFrontFace front_face;
    };

    // Pipeline depth-stencil

    enum class RHICompareOp {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };

    struct RHIDepthStencilDesc {
        bool depth_enable;
        bool depth_write_enable;
        RHICompareOp compare_op;
    };

    // Pipeline color blending

    enum class RHIBlendFactor {
        Zero,
        One,
        SrcColor,
        InvSrcColor,
        DstColor,
        InvDstColor,
        SrcAlpha,
        InvSrcAlpha,
        DstAlpha,
        InvDstAlpha,
        ConstantColor,
        InvConstantColor,
        ConstantAlpha,
        InvConstantAlpha,
        SrcAlphaSat,
        Src1Color,
        InvSrc1Color,
        Src1Alpha,
        InvSrc1Alpha,
    };

    enum class RHIBlendOp {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };

    using RHIColorWrite = std::uint8_t;
    struct RHIColorWriteFlags {
        static constexpr auto None = RHIColorWrite{0};
        static constexpr auto Red = RHIColorWrite{1};
        static constexpr auto Green = RHIColorWrite{2};
        static constexpr auto Blue = RHIColorWrite{4};
        static constexpr auto Alpha = RHIColorWrite{8};
        static constexpr auto All = Red | Green | Blue | Alpha;
    };

    struct RHIRenderTargetBlendDesc {
        bool blend_enable;
        RHIBlendFactor src_blend;
        RHIBlendFactor dst_blend;
        RHIBlendOp blend_op;
        RHIBlendFactor src_alpha_blend;
        RHIBlendFactor dst_alpha_blend;
        RHIBlendOp alpha_blend_op;
        RHIColorWrite color_write_mask;
    };

    struct RHIBlendDesc {
        std::span<const RHIRenderTargetBlendDesc> render_targets;
        std::array<float, 4> blend_constants;
    };

    struct RHIGraphicsPipelineDesc {
        std::span<const std::byte> VS;
        std::span<const std::byte> FS;
        std::span<const RHIVertexBinding> vertex_bindings;
        RHIInputAssemblyDesc input_assembly;
        RHIRasterizerDesc rasterizer;
        RHIDepthStencilDesc depth_stencil;
        RHIBlendDesc blend;
        std::span<const RHIFormat> rtv_formats;
        RHIFormat dsv_format;
    };
} // namespace orion
