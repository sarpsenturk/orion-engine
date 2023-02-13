#pragma once

#include "orion-renderer/config.h"
#include "orion-renderer/render_backend.h"

#include <memory> // std::unique_ptr

namespace orion
{
    class Renderer
    {
    public:
        explicit Renderer(const char* backend_module);

    private:
        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
    };
} // namespace orion
