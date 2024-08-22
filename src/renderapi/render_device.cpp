#include "orion/renderapi/render_device.h"

#include <spdlog/spdlog.h>

namespace orion
{
    std::unique_ptr<CommandQueue> RenderDevice::create_command_queue()
    {
        auto queue = create_command_queue_api();
        SPDLOG_DEBUG("Created command queue {}", fmt::ptr(queue.get()));
        return queue;
    }

    std::unique_ptr<Swapchain> RenderDevice::create_swapchain(const SwapchainDesc& desc)
    {
        auto swapchain = create_swapchain_api(desc);
        SPDLOG_DEBUG("Created swapchain {}", fmt::ptr(swapchain.get()));
        return swapchain;
    }

    std::unique_ptr<ShaderCompiler> RenderDevice::create_shader_compiler()
    {
        auto compiler = create_shader_compiler_api();
        SPDLOG_DEBUG("Created shader compiler {}", fmt::ptr(compiler.get()));
        return compiler;
    }
} // namespace orion
