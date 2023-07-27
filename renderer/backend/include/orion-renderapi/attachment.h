#pragma once

#include "types.h"

#include <span>

namespace orion
{
    struct AttachmentDesc {
        Format format;
    };

    struct AttachmentList {
        std::span<const AttachmentDesc> color_attachments;

        [[nodiscard]] auto attachment_count() const noexcept
        {
            return color_attachments.size();
        }
    };
} // namespace orion
