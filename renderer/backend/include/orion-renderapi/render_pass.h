#pragma once

#include "types.h"

#include <span>

namespace orion
{
    struct RenderPassAttachmentDesc {
        Format format;
        AttachmentLoadOp load_op;
        AttachmentStoreOp store_op;
        ImageLayout initial_layout;
        ImageLayout layout;
        ImageLayout final_layout;
    };

    struct RenderPassDesc {
        std::span<const RenderPassAttachmentDesc> color_attachments;
    };
} // namespace orion
