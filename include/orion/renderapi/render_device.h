#pragma once

#include "orion/renderapi/handle.h"
#include "orion/renderapi/pipeline.h"
#include "orion/renderapi/render_queue.h"
#include "orion/renderapi/shader.h"
#include "orion/renderapi/swapchain.h"

#include <memory>

namespace orion
{
    class RenderDevice
    {
    public:
        RenderDevice() = default;
        virtual ~RenderDevice() = default;

        std::unique_ptr<CommandQueue> create_command_queue();
        std::unique_ptr<Swapchain> create_swapchain(const SwapchainDesc& desc);
        std::unique_ptr<ShaderCompiler> create_shader_compiler();

        GraphicsPipelineHandle create_graphics_pipeline(const GraphicsPipelineDesc& desc);

        void destroy(GraphicsPipelineHandle pipeline);

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) = default;
        RenderDevice& operator=(RenderDevice&&) = default;

    private:
        virtual std::unique_ptr<CommandQueue> create_command_queue_api() = 0;
        virtual std::unique_ptr<Swapchain> create_swapchain_api(const SwapchainDesc& desc) = 0;
        virtual std::unique_ptr<ShaderCompiler> create_shader_compiler_api() = 0;

        virtual GraphicsPipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) = 0;

        virtual void destroy_api(GraphicsPipelineHandle pipeline) = 0;
    };
} // namespace orion
