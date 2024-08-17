#include "orion/renderapi/render_backend.h"

#include "d3d12/d3d12_backend.h"

namespace orion
{
    std::unique_ptr<RenderBackend> RenderBackend::create_builtin_d3d12()
    {
        return std::make_unique<D3D12Backend>();
    }
} // namespace orion
