#pragma once

#include "orion-utils/bitflag.h"

#include <array>
#include <cstdint>
#include <span>

namespace orion
{
    enum class BlendFactor {
        Zero,
        One,
        SrcColor,
        InvertedSrcColor,
        DstColor,
        InvertedDstColor,
        SrcAlpha,
        InvertedSrcAlpha,
        DstAlpha,
        InvertedDstAlpha
    };

    enum class BlendOp {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    ORION_BITFLAG(ColorComponentFlags, std::uint8_t){
        R = 0x1u,
        G = 0x2u,
        B = 0x4u,
        A = 0x8u,
        All = R | G | B | A,
    };

    struct BlendAttachmentDesc {
        bool enable_blend = false;
        BlendFactor src_blend = BlendFactor::Zero;
        BlendFactor dst_blend = BlendFactor::Zero;
        BlendOp blend_op = BlendOp::Add;
        BlendFactor src_blend_alpha = BlendFactor::Zero;
        BlendFactor dst_blend_alpha = BlendFactor::Zero;
        BlendOp blend_op_alpha = BlendOp::Add;
        ColorComponentFlags color_component_flags = ColorComponentFlags::All;
    };

    inline constexpr auto blend_attachment_disabled() noexcept
    {
        return BlendAttachmentDesc{.enable_blend = false, .color_component_flags = ColorComponentFlags::All};
    }

    inline constexpr auto blend_attachment_additive() noexcept
    {
        return BlendAttachmentDesc{
            .enable_blend = true,
            .src_blend = BlendFactor::One,
            .dst_blend = BlendFactor::DstAlpha,
            .blend_op = BlendOp::Add,
            .src_blend_alpha = BlendFactor::One,
            .dst_blend_alpha = BlendFactor::Zero,
            .blend_op_alpha = BlendOp::Add,
        };
    }

    inline constexpr auto blend_attachment_alphablend()
    {
        return BlendAttachmentDesc{
            .enable_blend = true,
            .src_blend = BlendFactor::SrcAlpha,
            .dst_blend = BlendFactor::InvertedSrcAlpha,
            .blend_op = BlendOp::Add,
            .src_blend_alpha = BlendFactor::One,
            .dst_blend_alpha = BlendFactor::Zero,
            .blend_op_alpha = BlendOp::Add,
        };
    }

    enum class LogicOp {
        NoOp,
        Clear,
        And,
        AndReverse,
        AndInverted,
        Nand,
        Or,
        OrReverse,
        OrInverted,
        Copy,
        CopyInverted,
        Xor,
        Nor,
        Equivalent,
        Set
    };

    struct ColorBlendDesc {
        bool enable_logic_op = false;
        LogicOp logic_op = LogicOp::NoOp;
        std::span<const BlendAttachmentDesc> attachments;
        std::array<float, 4> blend_constants = {1.f, 1.f, 1.f, 1.f};
    };
} // namespace orion
