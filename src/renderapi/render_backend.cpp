#include "orion/renderapi/render_backend.h"

#include "vulkan/vulkan_backend.h"

namespace orion
{
    std::unique_ptr<RenderBackend> RenderBackend::create_builtin_vulkan()
    {
        return std::make_unique<VulkanBackend>();
    }
} // namespace orion
