#pragma once

#include "buffer.h"
#include "command.h"
#include "handles.h"
#include "orion-renderapi/types.h"
#include "pipeline.h"
#include "render_context.h"
#include "shader.h"
#include "swapchain.h"

#include <spdlog/logger.h> // spdlog::logger

namespace orion
{
    // Forward declare window class
    class Window;

    class RenderDevice
    {
    public:
        RenderDevice(spdlog::logger* logger);
        virtual ~RenderDevice() = default;

        [[nodiscard]] virtual std::unique_ptr<RenderContext> create_render_context() = 0;

        [[nodiscard]] Swapchain create_swapchain(const Window& window, SwapchainDesc desc);
        [[nodiscard]] ShaderModule create_shader_module(const ShaderModuleDesc& desc);
        [[nodiscard]] GraphicsPipeline create_graphics_pipeline(const GraphicsPipelineDesc& desc);
        [[nodiscard]] GPUBuffer create_buffer(const GPUBufferDesc& desc);
        [[nodiscard]] CommandBuffer create_command_buffer(const CommandBufferDesc& desc, std::unique_ptr<CommandAllocator> allocator);

        void destroy(Swapchain swapchain);
        void destroy(ShaderModule shader_module);
        void destroy(GraphicsPipeline graphics_pipeline);
        void destroy(GPUBuffer buffer);
        template<typename T>
        void destroy(const CommandBuffer<T>& command_buffer)
        {
            destroy_api(command_buffer.handle());
        }

        [[nodiscard]] void* map(GPUBuffer buffer);
        void unmap(GPUBuffer buffer);

        [[nodiscard]] auto logger() const noexcept { return logger_; }

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) noexcept = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice& operator=(RenderDevice&&) noexcept = default;

    private:
        [[nodiscard]] virtual SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc, SwapchainHandle existing) = 0;
        [[nodiscard]] virtual ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc, ShaderModuleHandle existing) = 0;
        [[nodiscard]] virtual PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc, PipelineHandle existing) = 0;
        [[nodiscard]] virtual GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc, GPUBufferHandle existing) = 0;
        [[nodiscard]] virtual CommandBufferHandle create_command_buffer_api(const CommandBufferDesc& desc, CommandBufferHandle existing) = 0;

        virtual void destroy_api(SwapchainHandle swapchain_handle) = 0;
        virtual void destroy_api(ShaderModuleHandle shader_module_handle) = 0;
        virtual void destroy_api(PipelineHandle graphics_pipeline_handle) = 0;
        virtual void destroy_api(GPUBufferHandle buffer_handle) = 0;
        virtual void destroy_api(CommandBufferHandle command_buffer_handle) = 0;

        [[nodiscard]] virtual void* map_api(GPUBufferHandle buffer_handle) = 0;
        virtual void unmap_api(GPUBufferHandle buffer_handle) = 0;

        spdlog::logger* logger_;
    };
} // namespace orion
