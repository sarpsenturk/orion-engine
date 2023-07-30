#pragma once

#include "attachment.h"
#include "types.h"

#include <span>

namespace orion
{
    struct RenderPassDesc {
        AttachmentList attachments;
    };
} // namespace orion
