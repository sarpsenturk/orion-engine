#pragma once

#include "handles.h"
#include "types.h"

#include <span>

namespace orion
{
    // Only format is needed except when creating render passes
    struct AttachmentDesc {
        Format format;
        AttachmentLoadOp load_op = AttachmentLoadOp::DontCare;
        AttachmentStoreOp store_op = AttachmentStoreOp::DontCare;
        ImageLayout initial_layout = ImageLayout::Undefined;
        ImageLayout layout = ImageLayout::Undefined;
        ImageLayout final_layout = ImageLayout::Undefined;
    };

    struct AttachmentList {
        std::span<const AttachmentDesc> color_attachments;

        [[nodiscard]] auto attachment_count() const noexcept
        {
            return color_attachments.size();
        }
    };
} // namespace orion
