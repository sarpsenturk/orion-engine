#pragma once

#include "handles.h"

namespace orion
{
    class RenderPass
    {
    public:
        RenderPass() = default;
        RenderPass(RenderPassHandleRef handle);

    private:
        RenderPassHandleRef handle_;
    };
} // namespace orion
