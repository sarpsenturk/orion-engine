#include "orion-renderapi/render_device.h"

#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    Swapchain RenderDevice::create_swapchain(const Window& window, SwapchainDesc desc)
    {
        auto handle = create_swapchain_api(window, desc, SwapchainHandle::invalid_handle());
        SPDLOG_DEBUG("Created swapchain with handle {}", handle);
        return {make_handle_ref(handle), desc};
    }

    ShaderModule RenderDevice::create_shader_module(const ShaderModuleDesc& desc)
    {
        auto handle = create_shader_module_api(desc, ShaderModuleHandle::invalid_handle());
        SPDLOG_DEBUG("Created shader module with handle {}", handle);
        return {make_handle_ref(handle)};
    }
} // namespace orion
