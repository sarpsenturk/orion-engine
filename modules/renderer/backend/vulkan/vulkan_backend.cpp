#include "vulkan_backend.h"

extern "C" ORION_EXPORT orion::RenderBackend* create_render_backend()
{
    return new orion::vulkan::VulkanBackend();
}
