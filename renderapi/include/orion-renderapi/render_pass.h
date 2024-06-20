#pragma once

#include "orion-renderapi/handles.h"

#include "orion-math/vector/vector4.h"

#include <cstdint>

namespace orion
{
    using render_pass_attachment_t = std::int32_t;

    class RenderPass
    {
    public:
        RenderPass() = default;
        virtual ~RenderPass() = default;

        render_pass_attachment_t add_attachment();
        void clear(render_pass_attachment_t attachment, const Vector4_f& clear_color);

        void set_render_target(render_pass_attachment_t attachment, ImageViewHandle render_target);

    protected:
        RenderPass(const RenderPass&) = default;
        RenderPass(RenderPass&&) = default;
        RenderPass& operator=(const RenderPass&) = default;
        RenderPass& operator=(RenderPass&&) = default;

    private:
        virtual render_pass_attachment_t add_attachment_api() = 0;
        virtual void clear_api(render_pass_attachment_t attachment, const Vector4_f& clear_color) = 0;

        virtual void set_render_target_api(render_pass_attachment_t attachment, ImageViewHandle render_target) = 0;
    };
} // namespace orion
