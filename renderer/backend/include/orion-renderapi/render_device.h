#pragma once

#include "buffer.h"
#include "command.h"
#include "handles.h"
#include "orion-renderapi/types.h"
#include "pipeline.h"
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

        [[nodiscard]] SwapchainHandle create_swapchain(const Window& window, SwapchainDesc desc);
        [[nodiscard]] ShaderModuleHandle create_shader_module(const ShaderModuleDesc& desc);
        [[nodiscard]] PipelineHandle create_graphics_pipeline(const GraphicsPipelineDesc& desc);
        [[nodiscard]] GPUBufferHandle create_buffer(const GPUBufferDesc& desc);
        [[nodiscard]] CommandBuffer create_command_buffer(const CommandBufferDesc& desc, std::unique_ptr<CommandAllocator> allocator);

        void recreate(SwapchainHandle swapchain_handle, const SwapchainDesc& desc);

        void destroy(SwapchainHandle swapchain_handle);
        void destroy(ShaderModuleHandle shader_module_handle);
        void destroy(PipelineHandle pipeline_handle);
        void destroy(GPUBufferHandle buffer_handle);
        void destroy(CommandBufferHandle command_buffer_handle);
        void destroy(SubmissionHandle submission_handle);

        [[nodiscard]] void* map(GPUBufferHandle buffer_handle);
        void unmap(GPUBufferHandle buffer_handle);

        [[nodiscard]] SubmissionHandle submit(const CommandBuffer& command_buffer, SubmissionHandle existing = SubmissionHandle::invalid_handle());
        void wait(SubmissionHandle submission_handle);
        void present(SwapchainHandle swapchain_handle, SubmissionHandle wait);

        [[nodiscard]] auto logger() const noexcept { return logger_; }

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) noexcept = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice& operator=(RenderDevice&&) noexcept = default;

    private:
        [[nodiscard]] virtual SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc) = 0;
        [[nodiscard]] virtual ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc) = 0;
        [[nodiscard]] virtual PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) = 0;
        [[nodiscard]] virtual GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc) = 0;
        [[nodiscard]] virtual CommandBufferHandle create_command_buffer_api(const CommandBufferDesc& desc) = 0;

        virtual void recreate_api(SwapchainHandle swapchain_handle, const SwapchainDesc& desc) = 0;

        virtual void destroy_api(SwapchainHandle swapchain_handle) = 0;
        virtual void destroy_api(ShaderModuleHandle shader_module_handle) = 0;
        virtual void destroy_api(PipelineHandle graphics_pipeline_handle) = 0;
        virtual void destroy_api(GPUBufferHandle buffer_handle) = 0;
        virtual void destroy_api(CommandBufferHandle command_buffer_handle) = 0;
        virtual void destroy_api(SubmissionHandle submission_handle) = 0;

        [[nodiscard]] virtual void* map_api(GPUBufferHandle buffer_handle) = 0;
        virtual void unmap_api(GPUBufferHandle buffer_handle) = 0;

        [[nodiscard]] virtual SubmissionHandle submit_api(const CommandBuffer& command_buffer, SubmissionHandle existing) = 0;
        virtual void wait_api(SubmissionHandle submission_handle) = 0;
        virtual void present_api(SwapchainHandle swapchain_handle, SubmissionHandle wait) = 0;

        spdlog::logger* logger_;
    };
} // namespace orion
