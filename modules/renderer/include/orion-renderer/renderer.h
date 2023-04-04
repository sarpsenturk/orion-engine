#pragma once

#include "orion-renderapi/render_backend.h"
#include "orion-renderer/config.h"

#include <memory> // std::unique_ptr

namespace orion
{
    class Renderer
    {
    public:
        explicit Renderer(const char* backend_module);

        [[nodiscard]] auto device() const noexcept { return render_device_.get(); }
        [[nodiscard]] auto render_context() const noexcept { return render_context_.get(); }

    private:
        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
        std::unique_ptr<RenderContext> render_context_;
    };
} // namespace orion
