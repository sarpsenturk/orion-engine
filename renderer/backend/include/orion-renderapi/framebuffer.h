#pragma once

#include "attachment.h"

#include <orion-math/vector/vector2.h>

#include <span>

namespace orion
{
    struct FramebufferDesc {
        AttachmentList attachment_list;
        std::span<const ImageViewHandle> image_views = {};
        Vector2_u size = {};
    };
} // namespace orion
