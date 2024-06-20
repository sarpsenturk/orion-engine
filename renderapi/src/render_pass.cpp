#include "orion-renderapi/render_pass.h"

namespace orion
{
    render_pass_attachment_t RenderPass::add_attachment()
    {
        return add_attachment_api();
    }

    void RenderPass::clear(render_pass_attachment_t attachment, const Vector4_f& clear_color)
    {
        return clear_api(attachment, clear_color);
    }

    void RenderPass::set_render_target(render_pass_attachment_t attachment, ImageViewHandle render_target)
    {
        return set_render_target_api(attachment, render_target);
    }
} // namespace orion
