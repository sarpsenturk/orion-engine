#pragma once

#include "buffer.h"
#include "command.h"
#include "descriptor.h"
#include "handles.h"
#include "pipeline.h"
#include "render_pass.h"
#include "render_target.h"
#include "shader.h"
#include "swapchain.h"
#include "types.h"

#include <spdlog/logger.h> // spdlog::logger

namespace orion
{
    // Forward declare window class
    class Window;

    struct SubmitDesc {
        const CommandBuffer* command_buffer;
        CommandQueueType queue_type;
        SubmissionHandle existing = SubmissionHandle::invalid_handle();
    };

    class RenderDevice
    {
    public:
        RenderDevice(spdlog::logger* logger);
        virtual ~RenderDevice() = default;

        [[nodiscard]] SwapchainHandle create_swapchain(const Window& window, const SwapchainDesc& desc);
        [[nodiscard]] RenderPassHandle create_render_pass(const RenderPassDesc& desc);
        [[nodiscard]] RenderTargetHandle create_render_target(SwapchainHandle swapchain, const RenderTargetDesc& desc);
        [[nodiscard]] ShaderModuleHandle create_shader_module(const ShaderModuleDesc& desc);
        [[nodiscard]] PipelineHandle create_graphics_pipeline(const GraphicsPipelineDesc& desc);
        [[nodiscard]] GPUBufferHandle create_buffer(const GPUBufferDesc& desc);
        [[nodiscard]] CommandPoolHandle create_command_pool(const CommandPoolDesc& desc);
        [[nodiscard]] CommandBufferHandle create_command_buffer(const CommandBufferDesc& desc);
        [[nodiscard]] DescriptorPoolHandle create_descriptor_pool(const DescriptorPoolDesc& desc);
        [[nodiscard]] DescriptorSetHandle create_descriptor_set(const DescriptorSetDesc& desc);

        void recreate(SwapchainHandle swapchain_handle, const SwapchainDesc& desc);
        void recreate(RenderTargetHandle render_target, SwapchainHandle swapchain, const RenderTargetDesc& desc);

        void destroy(SwapchainHandle swapchain_handle);
        void destroy(RenderPassHandle render_pass_handle);
        void destroy(RenderTargetHandle render_target_handle);
        void destroy(ShaderModuleHandle shader_module_handle);
        void destroy(PipelineHandle pipeline_handle);
        void destroy(GPUBufferHandle buffer_handle);
        void destroy(CommandPoolHandle command_pool_handle);
        void destroy(CommandBufferHandle command_buffer_handle);
        void destroy(SubmissionHandle submission_handle);
        void destroy(DescriptorPoolHandle descriptor_pool_handle);
        void destroy(DescriptorSetHandle descriptor_set_handle);

        [[nodiscard]] void* map(GPUBufferHandle buffer_handle);
        void unmap(GPUBufferHandle buffer_handle);

        [[nodiscard]] SubmissionHandle submit(const SubmitDesc& desc);
        void wait(SubmissionHandle submission_handle);
        void present(SwapchainHandle swapchain_handle, SubmissionHandle wait);

        void update_descriptors(const DescriptorUpdate& update);

        [[nodiscard]] auto logger() const noexcept { return logger_; }

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) noexcept = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice& operator=(RenderDevice&&) noexcept = default;

    private:
        [[nodiscard]] virtual SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc) = 0;
        [[nodiscard]] virtual RenderPassHandle create_render_pass_api(const RenderPassDesc& desc) = 0;
        [[nodiscard]] virtual RenderTargetHandle create_render_target_api(SwapchainHandle swapchain, const RenderTargetDesc& desc) = 0;
        [[nodiscard]] virtual ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc) = 0;
        [[nodiscard]] virtual PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) = 0;
        [[nodiscard]] virtual GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc) = 0;
        [[nodiscard]] virtual CommandPoolHandle create_command_pool_api(const CommandPoolDesc& desc) = 0;
        [[nodiscard]] virtual CommandBufferHandle create_command_buffer_api(const CommandBufferDesc& desc) = 0;
        [[nodiscard]] virtual DescriptorPoolHandle create_descriptor_pool_api(const DescriptorPoolDesc& desc) = 0;
        [[nodiscard]] virtual DescriptorSetHandle create_descriptor_set_api(const DescriptorSetDesc& desc) = 0;

        virtual void recreate_api(SwapchainHandle swapchain_handle, const SwapchainDesc& desc) = 0;
        virtual void recreate_api(RenderTargetHandle render_target, SwapchainHandle swapchain, const RenderTargetDesc& desc) = 0;

        virtual void destroy_api(SwapchainHandle swapchain_handle) = 0;
        virtual void destroy_api(RenderPassHandle render_pass_handle) = 0;
        virtual void destroy_api(RenderTargetHandle render_target_handle) = 0;
        virtual void destroy_api(ShaderModuleHandle shader_module_handle) = 0;
        virtual void destroy_api(PipelineHandle graphics_pipeline_handle) = 0;
        virtual void destroy_api(GPUBufferHandle buffer_handle) = 0;
        virtual void destroy_api(CommandBufferHandle command_buffer_handle) = 0;
        virtual void destroy_api(CommandPoolHandle command_pool_handle) = 0;
        virtual void destroy_api(SubmissionHandle submission_handle) = 0;
        virtual void destroy_api(DescriptorPoolHandle descriptor_pool_handle) = 0;
        virtual void destroy_api(DescriptorSetHandle descriptor_set_handle) = 0;

        [[nodiscard]] virtual void* map_api(GPUBufferHandle buffer_handle) = 0;
        virtual void unmap_api(GPUBufferHandle buffer_handle) = 0;

        [[nodiscard]] virtual SubmissionHandle submit_api(const SubmitDesc& desc) = 0;
        virtual void wait_api(SubmissionHandle submission_handle) = 0;
        virtual void present_api(SwapchainHandle swapchain_handle, SubmissionHandle wait) = 0;

        virtual void update_descriptors_api(const DescriptorUpdate& update) = 0;

        spdlog::logger* logger_;
    };
} // namespace orion
