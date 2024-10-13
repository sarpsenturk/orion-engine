#include "orion/renderapi/render_backend.hpp"

#include "vulkan/vulkan_backend.hpp"

#include "orion/platform.hpp"

#include <spdlog/spdlog.h>

namespace orion
{
    std::unique_ptr<RenderBackend> RenderBackend::create_builtin_vulkan()
    {
        return std::make_unique<VulkanBackend>();
    }

    std::unique_ptr<RenderBackend> RenderBackend::create_default()
    {
        return create_builtin_vulkan();
    }

    std::vector<GraphicsAdapter> RenderBackend::get_adapters()
    {
        return get_adapters_api();
    }

    std::unique_ptr<RenderDevice> RenderBackend::create_device(std::size_t adapter_index)
    {
        auto device = create_device_api(adapter_index);
        SPDLOG_DEBUG("Created RenderDevice {}", fmt::ptr(device.get()));
        return device;
    }
} // namespace orion
