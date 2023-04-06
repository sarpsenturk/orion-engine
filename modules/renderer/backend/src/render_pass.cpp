#include "orion-renderapi/render_pass.h"

namespace orion
{
    RenderPass::RenderPass(RenderPassHandleRef handle)
        : handle_(std::move(handle))
    {
    }
} // namespace orion
