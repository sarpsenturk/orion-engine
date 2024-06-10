#pragma once

#include "orion-renderapi/format.h"
#include "orion-renderapi/image.h"
#include "orion-renderapi/types.h"

#include "orion-math/vector/vector3.h"

namespace orion
{
    enum class AttachmentLoadOp {
        Load,
        Clear,
        DontCare
    };

    enum class AttachmentStoreOp {
        Store,
        DontCare
    };

    // Only format is needed except when creating render passes
    struct AttachmentDesc {
        Format format;
        AttachmentLoadOp load_op = AttachmentLoadOp::DontCare;
        AttachmentStoreOp store_op = AttachmentStoreOp::DontCare;
        ImageLayout initial_layout = ImageLayout::Undefined;
        ImageLayout layout = ImageLayout::Undefined;
        ImageLayout final_layout = ImageLayout::Undefined;
    };

    struct RenderPassDesc {
        PipelineBindPoint bind_point;
        std::span<const AttachmentDesc> color_attachments;
    };

    struct FramebufferDesc {
        RenderPassHandle render_pass;
        std::span<const ImageViewHandle> image_views;
        Vector2_u size;
    };
} // namespace orion
