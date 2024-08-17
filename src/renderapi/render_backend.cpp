#include "orion/renderapi/render_backend.h"

#include "d3d12/d3d12_backend.h"
#include "vulkan/vulkan_backend.h"

#include "orion/platform.h"

namespace orion
{
    std::unique_ptr<RenderBackend> RenderBackend::create_builtin_vulkan()
    {
        return std::make_unique<VulkanBackend>();
    }

    std::unique_ptr<RenderBackend> RenderBackend::create_builtin_d3d12()
    {
        return std::make_unique<D3D12Backend>();
    }

    std::unique_ptr<RenderBackend> RenderBackend::create()
    {
#ifdef ORION_PLATFORM_WIN32
        return create_builtin_d3d12();
#else
        return create_builtin_vulkan();
#endif
    }
} // namespace orion
