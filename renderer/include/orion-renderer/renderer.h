#pragma once

#include "orion-core/dyn_lib.h"
#include "orion-core/platform.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_device.h"

#include "orion-math/vector/vector2.h"

namespace orion
{
    struct RendererDesc {
        const char* backend = nullptr;
        Vector2_u render_size;
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

    private:
        Module render_backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
    };
} // namespace orion
