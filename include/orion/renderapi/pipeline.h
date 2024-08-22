#pragma once

#include "orion/renderapi/format.h"

#include <array>
#include <cstddef>
#include <span>

namespace orion
{
    struct VertexAttribute {
        const char* name;
        Format format;
    };

    enum class PrimitiveTopology {
        Point,
        Line,
        Triangle,
    };

    enum class FillMode {
        Solid,
        Wireframe,
    };

    enum class CullMode {
        None,
        Back,
        Front,
    };

    enum class FrontFace {
        CounterClockWise,
        ClockWise,
    };

    struct RasterizerDesc {
        FillMode fill_mode;
        CullMode cull_mode;
        FrontFace front_face;
    };

    enum class Blend {
        Zero,
        One,
    };

    enum class BlendOp {
        Add,
        Subtract,
    };

    enum class ColorWriteFlags : std::uint8_t {
        None = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        All = Red | Blue | Green,
    };

    // TODO: Allow declaring bit flags automatically
    constexpr ColorWriteFlags operator&(ColorWriteFlags lhs, ColorWriteFlags rhs)
    {
        return static_cast<ColorWriteFlags>(static_cast<std::underlying_type_t<ColorWriteFlags>>(lhs) & static_cast<std::underlying_type_t<ColorWriteFlags>>(rhs));
    }

    struct RenderTargetBlendDesc {
        bool blend_enable;
        Blend src_blend;
        Blend dst_blend;
        BlendOp blend_op;
        Blend src_blend_alpha;
        Blend dst_blend_alpha;
        BlendOp blend_op_alpha;
        ColorWriteFlags color_write_mask;
    };

    struct BlendDesc {
        std::span<const RenderTargetBlendDesc> render_targets;
        std::array<float, 4> blend_constants;
    };

    struct GraphicsPipelineDesc {
        std::span<const std::byte> vertex_shader;
        std::span<const std::byte> pixel_shader;
        std::span<const VertexAttribute> vertex_attributes;
        PrimitiveTopology primitive_topology;
        RasterizerDesc rasterizer;
        BlendDesc blend;
        std::span<const Format> render_target_formats;
    };
} // namespace orion
