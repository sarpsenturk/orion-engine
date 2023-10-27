#pragma once

#include "command.h"
#include "handles.h"
#include "swapchain.h"
#include "types.h"

#include <memory>
#include <span>

#include <spdlog/logger.h>

namespace orion
{
    // Forward declarations
    class Window;
    class RenderDevice;

    template<typename Tag>
    struct ResourceDeleter {
        using pointer = Handle<Tag>;

        RenderDevice* device;

        void operator()(pointer handle) const
        {
            device->destroy(handle);
        }
    };

    template<typename Tag>
    using unique_device_resource = std::unique_ptr<Handle<Tag>, ResourceDeleter<Tag>>;

    using UniqueRenderPass = unique_device_resource<RenderPassHandle_tag>;
    using UniqueFramebuffer = unique_device_resource<FramebufferHandle_tag>;
    using UniqueShaderModule = unique_device_resource<ShaderModuleHandle_tag>;
    using UniquePipeline = unique_device_resource<PipelineHandle_tag>;
    using UniqueGPUBuffer = unique_device_resource<GPUBufferHandle_tag>;
    using UniqueCommandPool = unique_device_resource<CommandPoolHandle_tag>;
    using UniqueCommandBuffer = unique_device_resource<CommandBufferHandle_tag>;
    using UniqueDescriptorPool = unique_device_resource<DescriptorPoolHandle_tag>;
    using UniqueDescriptorSet = unique_device_resource<DescriptorSetHandle_tag>;
    using UniqueImage = unique_device_resource<ImageHandle_tag>;
    using UniqueImageView = unique_device_resource<ImageViewHandle_tag>;
    using UniqueSampler = unique_device_resource<SamplerHandle_tag>;
    using UniqueGPUJob = unique_device_resource<GPUJobHandle_tag>;

    class RenderDevice
    {
    public:
        explicit RenderDevice(spdlog::logger* logger);
        virtual ~RenderDevice() = default;

        [[nodiscard]] virtual ShaderObjectType shader_object_type() const noexcept = 0;

        [[nodiscard]] std::unique_ptr<Swapchain> create_swapchain(const SwapchainDesc& desc);
        [[nodiscard]] RenderPassHandle create_render_pass(const RenderPassDesc& desc);
        [[nodiscard]] FramebufferHandle create_framebuffer(const FramebufferDesc& desc);
        [[nodiscard]] ShaderModuleHandle create_shader_module(const ShaderModuleDesc& desc);
        [[nodiscard]] PipelineHandle create_graphics_pipeline(const GraphicsPipelineDesc& desc);
        [[nodiscard]] GPUBufferHandle create_buffer(const GPUBufferDesc& desc);
        [[nodiscard]] CommandPoolHandle create_command_pool(const CommandPoolDesc& desc);
        [[nodiscard]] CommandBufferHandle create_command_buffer(const CommandBufferDesc& desc);
        [[nodiscard]] DescriptorPoolHandle create_descriptor_pool(const DescriptorPoolDesc& desc);
        [[nodiscard]] DescriptorSetHandle create_descriptor_set(const DescriptorSetDesc& desc);
        [[nodiscard]] ImageHandle create_image(const ImageDesc& desc);
        [[nodiscard]] ImageViewHandle create_image_view(const ImageViewDesc& desc);
        [[nodiscard]] SamplerHandle create_sampler(const SamplerDesc& desc);
        [[nodiscard]] GPUJobHandle create_job(const GPUJobDesc& desc);

        [[nodiscard]] RenderPassHandle create(RenderPassHandle_tag, const RenderPassDesc& desc) { return create_render_pass(desc); }
        [[nodiscard]] FramebufferHandle create(FramebufferHandle_tag, const FramebufferDesc& desc) { return create_framebuffer(desc); }
        [[nodiscard]] ShaderModuleHandle create(ShaderModuleHandle_tag, const ShaderModuleDesc& desc) { return create_shader_module(desc); }
        [[nodiscard]] PipelineHandle create(PipelineHandle_tag, const GraphicsPipelineDesc& desc) { return create_graphics_pipeline(desc); }
        [[nodiscard]] GPUBufferHandle create(GPUBufferHandle_tag, const GPUBufferDesc& desc) { return create_buffer(desc); }
        [[nodiscard]] CommandPoolHandle create(CommandPoolHandle_tag, const CommandPoolDesc& desc) { return create_command_pool(desc); }
        [[nodiscard]] CommandBufferHandle create(CommandBufferHandle_tag, const CommandBufferDesc& desc) { return create_command_buffer(desc); }
        [[nodiscard]] DescriptorPoolHandle create(DescriptorPoolHandle_tag, const DescriptorPoolDesc& desc) { return create_descriptor_pool(desc); }
        [[nodiscard]] DescriptorSetHandle create(DescriptorSetHandle_tag, const DescriptorSetDesc& desc) { return create_descriptor_set(desc); }
        [[nodiscard]] ImageHandle create(ImageHandle_tag, const ImageDesc& desc) { return create_image(desc); }
        [[nodiscard]] ImageViewHandle create(ImageViewHandle_tag, const ImageViewDesc& desc) { return create_image_view(desc); }
        [[nodiscard]] SamplerHandle create(SamplerHandle_tag, const SamplerDesc& desc) { return create_sampler(desc); }
        [[nodiscard]] GPUJobHandle create(GPUJobHandle_tag, const GPUJobDesc& desc) { return create_job(desc); }

        template<typename Tag, typename... Args>
        auto make_unique(Tag tag, Args&&... args)
        {
            return unique_device_resource<Tag>{create(tag, std::forward<Args>(args)...), {this}};
        }

        template<typename Tag>
        auto to_unique(Handle<Tag> handle)
        {
            return unique_device_resource<Tag>{handle, {this}};
        }

        void destroy(RenderPassHandle render_pass_handle);
        void destroy(FramebufferHandle framebuffer_handle);
        void destroy(ShaderModuleHandle shader_module_handle);
        void destroy(PipelineHandle pipeline_handle);
        void destroy(GPUBufferHandle buffer_handle);
        void destroy(CommandPoolHandle command_pool_handle);
        void destroy(CommandBufferHandle command_buffer_handle);
        void destroy(DescriptorPoolHandle descriptor_pool_handle);
        void destroy(DescriptorSetHandle descriptor_set_handle);
        void destroy(ImageHandle image_handle);
        void destroy(ImageViewHandle image_view_handle);
        void destroy(SamplerHandle sampler_handle);
        void destroy(GPUJobHandle job_handle);

        [[nodiscard]] void* map(GPUBufferHandle buffer_handle);
        void unmap(GPUBufferHandle buffer_handle);

        void reset_command_pool(CommandPoolHandle command_pool);
        void reset_command_buffer(CommandBufferHandle command_buffer);
        void compile_commands(CommandBufferHandle command_buffer, std::span<const CommandPacket> commands);

        void wait_for_job(GPUJobHandle job_handle);
        void wait_queue_idle(CommandQueueType queue_type);
        void wait_idle();

        void update_descriptor_sets(std::span<const DescriptorSetUpdate> updates);

        [[nodiscard]] auto logger() const noexcept { return logger_; }

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) noexcept = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice& operator=(RenderDevice&&) noexcept = default;

    private:
        [[nodiscard]] virtual std::unique_ptr<Swapchain> create_swapchain_api(const SwapchainDesc& desc) = 0;
        [[nodiscard]] virtual RenderPassHandle create_render_pass_api(const RenderPassDesc& desc) = 0;
        [[nodiscard]] virtual FramebufferHandle create_framebuffer_api(const FramebufferDesc& desc) = 0;
        [[nodiscard]] virtual ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc) = 0;
        [[nodiscard]] virtual PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) = 0;
        [[nodiscard]] virtual GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc) = 0;
        [[nodiscard]] virtual CommandPoolHandle create_command_pool_api(const CommandPoolDesc& desc) = 0;
        [[nodiscard]] virtual CommandBufferHandle create_command_buffer_api(const CommandBufferDesc& desc) = 0;
        [[nodiscard]] virtual DescriptorPoolHandle create_descriptor_pool_api(const DescriptorPoolDesc& desc) = 0;
        [[nodiscard]] virtual DescriptorSetHandle create_descriptor_set_api(const DescriptorSetDesc& desc) = 0;
        [[nodiscard]] virtual ImageHandle create_image_api(const ImageDesc& desc) = 0;
        [[nodiscard]] virtual ImageViewHandle create_image_view_api(const ImageViewDesc& desc) = 0;
        [[nodiscard]] virtual SamplerHandle create_sampler_api(const SamplerDesc& desc) = 0;
        [[nodiscard]] virtual GPUJobHandle create_job_api(const GPUJobDesc& desc) = 0;

        virtual void destroy_api(RenderPassHandle render_pass_handle) = 0;
        virtual void destroy_api(FramebufferHandle framebuffer_handle) = 0;
        virtual void destroy_api(ShaderModuleHandle shader_module_handle) = 0;
        virtual void destroy_api(PipelineHandle graphics_pipeline_handle) = 0;
        virtual void destroy_api(GPUBufferHandle buffer_handle) = 0;
        virtual void destroy_api(CommandBufferHandle command_buffer_handle) = 0;
        virtual void destroy_api(CommandPoolHandle command_pool_handle) = 0;
        virtual void destroy_api(DescriptorPoolHandle descriptor_pool_handle) = 0;
        virtual void destroy_api(DescriptorSetHandle descriptor_set_handle) = 0;
        virtual void destroy_api(ImageHandle image_handle) = 0;
        virtual void destroy_api(ImageViewHandle image_view_handle) = 0;
        virtual void destroy_api(SamplerHandle sampler_handle) = 0;
        virtual void destroy_api(GPUJobHandle job_handle) = 0;

        [[nodiscard]] virtual void* map_api(GPUBufferHandle buffer_handle) = 0;
        virtual void unmap_api(GPUBufferHandle buffer_handle) = 0;

        virtual void reset_command_pool_api(CommandPoolHandle command_pool) = 0;
        virtual void reset_command_buffer_api(CommandBufferHandle command_buffer) = 0;
        virtual void compile_commands_api(CommandBufferHandle command_buffer, std::span<const CommandPacket> commands) = 0;

        virtual void wait_for_job_api(GPUJobHandle job_handle) = 0;
        virtual void wait_queue_idle_api(CommandQueueType queue_type) = 0;
        virtual void wait_idle_api() = 0;

        virtual void update_descriptor_sets_api(std::span<const DescriptorSetUpdate> updates) = 0;

        spdlog::logger* logger_;
    };
} // namespace orion
