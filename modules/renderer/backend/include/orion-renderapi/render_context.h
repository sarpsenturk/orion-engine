#pragma once

namespace orion
{
    class RenderContext
    {
    public:
        RenderContext() = default;
        virtual ~RenderContext() = default;

    protected:
        RenderContext(const RenderContext&) = default;
        RenderContext(RenderContext&&) noexcept = default;
        RenderContext& operator=(const RenderContext&) = default;
        RenderContext& operator=(RenderContext&&) noexcept = default;
    };
} // namespace orion
