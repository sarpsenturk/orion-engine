#pragma once

#include "handles.h"

#include <orion-math/vector/vector2.h>

#include <span>

namespace orion
{
    struct FramebufferDesc {
        RenderPassHandle render_pass = RenderPassHandle::invalid_handle();
        std::span<const AttachmentHandle> attachments = {};
        Vector2_u size = {};
    };
} // namespace orion
