#pragma once

#include "buffer.h"
#include "command.h"
#include "descriptor.h"
#include "framebuffer.h"
#include "handles.h"
#include "image.h"
#include "pipeline.h"
#include "render_pass.h"
#include "shader.h"
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

    struct SubmitDesc {
        CommandQueueType queue_type = CommandQueueType::Any;
        std::span<const CommandBufferHandle> command_buffers = {};
        std::span<const SemaphoreHandle> wait_semaphores = {};
        std::span<const PipelineStage> wait_stages = {};
        std::span<const SemaphoreHandle> signal_semaphores = {};
        FenceHandle fence = FenceHandle::invalid_handle();
    };

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

    using UniqueSurface = unique_device_resource<SurfaceHandle_tag>;
    using UniqueSwapchain = unique_device_resource<SwapchainHandle_tag>;
    using UniqueRenderPass = unique_device_resource<RenderPassHandle_tag>;
    using UniqueFramebuffer = unique_device_resource<FramebufferHandle_tag>;
    using UniqueShaderModule = unique_device_resource<ShaderModuleHandle_tag>;
    using UniquePipeline = unique_device_resource<PipelineHandle_tag>;
    using UniqueGPUBuffer = unique_device_resource<GPUBufferHandle_tag>;
    using UniqueCommandPool = unique_device_resource<CommandPoolHandle_tag>;
    using UniqueCommandBuffer = unique_device_resource<CommandBufferHandle_tag>;
    using UniqueDescriptorPool = unique_device_resource<DescriptorPoolHandle_tag>;
    using UniqueDescriptorSet = unique_device_resource<DescriptorSetHandle_tag>;
    using UniqueSemaphore = unique_device_resource<SemaphoreHandle_tag>;
    using UniqueFence = unique_device_resource<FenceHandle_tag>;
    using UniqueImage = unique_device_resource<ImageHandle_tag>;
    using UniqueImageView = unique_device_resource<ImageViewHandle_tag>;

    class RenderDevice
    {
    public:
        explicit RenderDevice(spdlog::logger* logger);
        virtual ~RenderDevice() = default;

        [[nodiscard]] virtual ShaderObjectType shader_object_type() const noexcept = 0;

        [[nodiscard]] SurfaceHandle create_surface(const Window& window);
        [[nodiscard]] SwapchainHandle create_swapchain(const SwapchainDesc& desc);
        [[nodiscard]] RenderPassHandle create_render_pass(const RenderPassDesc& desc);
        [[nodiscard]] FramebufferHandle create_framebuffer(const FramebufferDesc& desc);
        [[nodiscard]] ShaderModuleHandle create_shader_module(const ShaderModuleDesc& desc);
        [[nodiscard]] PipelineHandle create_graphics_pipeline(const GraphicsPipelineDesc& desc);
        [[nodiscard]] GPUBufferHandle create_buffer(const GPUBufferDesc& desc);
        [[nodiscard]] CommandPoolHandle create_command_pool(const CommandPoolDesc& desc);
        [[nodiscard]] CommandBufferHandle create_command_buffer(const CommandBufferDesc& desc);
        [[nodiscard]] DescriptorPoolHandle create_descriptor_pool(const DescriptorPoolDesc& desc);
        [[nodiscard]] DescriptorSetHandle create_descriptor_set(const DescriptorSetDesc& desc);
        [[nodiscard]] SemaphoreHandle create_semaphore();
        [[nodiscard]] FenceHandle create_fence(bool create_signaled);
        [[nodiscard]] ImageHandle create_image(const ImageDesc& desc);
        [[nodiscard]] ImageViewHandle create_image_view(const ImageViewDesc& desc);

        [[nodiscard]] SurfaceHandle create(SurfaceHandle_tag, const Window& window) { return create_surface(window); }
        [[nodiscard]] SwapchainHandle create(SwapchainHandle_tag, const SwapchainDesc& desc) { return create_swapchain(desc); }
        [[nodiscard]] RenderPassHandle create(RenderPassHandle_tag, const RenderPassDesc& desc) { return create_render_pass(desc); }
        [[nodiscard]] FramebufferHandle create(FramebufferHandle_tag, const FramebufferDesc& desc) { return create_framebuffer(desc); }
        [[nodiscard]] ShaderModuleHandle create(ShaderModuleHandle_tag, const ShaderModuleDesc& desc) { return create_shader_module(desc); }
        [[nodiscard]] PipelineHandle create(PipelineHandle_tag, const GraphicsPipelineDesc& desc) { return create_graphics_pipeline(desc); }
        [[nodiscard]] GPUBufferHandle create(GPUBufferHandle_tag, const GPUBufferDesc& desc) { return create_buffer(desc); }
        [[nodiscard]] CommandPoolHandle create(CommandPoolHandle_tag, const CommandPoolDesc& desc) { return create_command_pool(desc); }
        [[nodiscard]] CommandBufferHandle create(CommandBufferHandle_tag, const CommandBufferDesc& desc) { return create_command_buffer(desc); }
        [[nodiscard]] DescriptorPoolHandle create(DescriptorPoolHandle_tag, const DescriptorPoolDesc& desc) { return create_descriptor_pool(desc); }
        [[nodiscard]] DescriptorSetHandle create(DescriptorSetHandle_tag, const DescriptorSetDesc& desc) { return create_descriptor_set(desc); }
        [[nodiscard]] SemaphoreHandle create(SemaphoreHandle_tag) { return create_semaphore(); }
        [[nodiscard]] FenceHandle create(FenceHandle_tag, bool create_signaled) { return create_fence(create_signaled); }
        [[nodiscard]] ImageHandle create(ImageHandle_tag, const ImageDesc& desc) { return create_image(desc); }
        [[nodiscard]] ImageViewHandle create(ImageViewHandle_tag, const ImageViewDesc& desc) { return create_image_view(desc); }

        template<typename Tag, typename... Args>
        auto make_unique(Tag tag, Args&&... args)
        {
            return unique_device_resource<Tag>{create(tag, std::forward<Args>(args)...), {this}};
        }

        void destroy(SurfaceHandle surface_handle);
        void destroy(SwapchainHandle swapchain_handle);
        void destroy(RenderPassHandle render_pass_handle);
        void destroy(FramebufferHandle framebuffer_handle);
        void destroy(ShaderModuleHandle shader_module_handle);
        void destroy(PipelineHandle pipeline_handle);
        void destroy(GPUBufferHandle buffer_handle);
        void destroy(CommandPoolHandle command_pool_handle);
        void destroy(CommandBufferHandle command_buffer_handle);
        void destroy(DescriptorPoolHandle descriptor_pool_handle);
        void destroy(DescriptorSetHandle descriptor_set_handle);
        void destroy(SemaphoreHandle semaphore_handle);
        void destroy(FenceHandle fence_handle);
        void destroy(ImageHandle image_handle);
        void destroy(ImageViewHandle image_view_handle);

        [[nodiscard]] void* map(GPUBufferHandle buffer_handle);
        void unmap(GPUBufferHandle buffer_handle);

        void reset_command_pool(CommandPoolHandle command_pool);
        void begin_command_buffer(CommandBufferHandle command_buffer, const CommandBufferBeginDesc& desc);
        void end_command_buffer(CommandBufferHandle command_buffer);
        void reset_command_buffer(CommandBufferHandle command_buffer);
        void compile_commands(CommandBufferHandle command_buffer, std::span<const CommandPacket> commands);

        void submit(const SubmitDesc& desc);
        void submit_immediate(const SubmitDesc& desc);
        void present(const SwapchainPresentDesc& desc);

        void wait_for_fence(FenceHandle fence);
        void wait_queue_idle(CommandQueueType queue_type);
        void wait_idle();

        void bind_descriptor(const DescriptorBufferBinding& binding);

        std::uint32_t acquire_next_image(SwapchainHandle swapchain, SemaphoreHandle semaphore, FenceHandle fence);
        ImageHandle get_swapchain_image(SwapchainHandle swapchain, std::uint32_t image_index);

        [[nodiscard]] auto logger() const noexcept { return logger_; }

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) noexcept = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice& operator=(RenderDevice&&) noexcept = default;

    private:
        [[nodiscard]] virtual SurfaceHandle create_surface_api(const Window& window) = 0;
        [[nodiscard]] virtual SwapchainHandle create_swapchain_api(const SwapchainDesc& desc) = 0;
        [[nodiscard]] virtual RenderPassHandle create_render_pass_api(const RenderPassDesc& desc) = 0;
        [[nodiscard]] virtual FramebufferHandle create_framebuffer_api(const FramebufferDesc& desc) = 0;
        [[nodiscard]] virtual ShaderModuleHandle create_shader_module_api(const ShaderModuleDesc& desc) = 0;
        [[nodiscard]] virtual PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) = 0;
        [[nodiscard]] virtual GPUBufferHandle create_buffer_api(const GPUBufferDesc& desc) = 0;
        [[nodiscard]] virtual CommandPoolHandle create_command_pool_api(const CommandPoolDesc& desc) = 0;
        [[nodiscard]] virtual CommandBufferHandle create_command_buffer_api(const CommandBufferDesc& desc) = 0;
        [[nodiscard]] virtual DescriptorPoolHandle create_descriptor_pool_api(const DescriptorPoolDesc& desc) = 0;
        [[nodiscard]] virtual DescriptorSetHandle create_descriptor_set_api(const DescriptorSetDesc& desc) = 0;
        [[nodiscard]] virtual SemaphoreHandle create_semaphore_api() = 0;
        [[nodiscard]] virtual FenceHandle create_fence_api(bool create_signaled) = 0;
        [[nodiscard]] virtual ImageHandle create_image_api(const ImageDesc& desc) = 0;
        [[nodiscard]] virtual ImageViewHandle create_image_view_api(const ImageViewDesc& desc) = 0;

        virtual void destroy_api(SurfaceHandle surface_handle) = 0;
        virtual void destroy_api(SwapchainHandle swapchain_handle) = 0;
        virtual void destroy_api(RenderPassHandle render_pass_handle) = 0;
        virtual void destroy_api(FramebufferHandle framebuffer_handle) = 0;
        virtual void destroy_api(ShaderModuleHandle shader_module_handle) = 0;
        virtual void destroy_api(PipelineHandle graphics_pipeline_handle) = 0;
        virtual void destroy_api(GPUBufferHandle buffer_handle) = 0;
        virtual void destroy_api(CommandBufferHandle command_buffer_handle) = 0;
        virtual void destroy_api(CommandPoolHandle command_pool_handle) = 0;
        virtual void destroy_api(DescriptorPoolHandle descriptor_pool_handle) = 0;
        virtual void destroy_api(DescriptorSetHandle descriptor_set_handle) = 0;
        virtual void destroy_api(SemaphoreHandle semaphore_handle) = 0;
        virtual void destroy_api(FenceHandle fence_handle) = 0;
        virtual void destroy_api(ImageHandle image_handle) = 0;
        virtual void destroy_api(ImageViewHandle image_view_handle) = 0;

        [[nodiscard]] virtual void* map_api(GPUBufferHandle buffer_handle) = 0;
        virtual void unmap_api(GPUBufferHandle buffer_handle) = 0;

        virtual void reset_command_pool_api(CommandPoolHandle command_pool) = 0;
        virtual void begin_command_buffer_api(CommandBufferHandle command_buffer, const CommandBufferBeginDesc& desc) = 0;
        virtual void end_command_buffer_api(CommandBufferHandle command_buffer) = 0;
        virtual void reset_command_buffer_api(CommandBufferHandle command_buffer) = 0;
        virtual void compile_commands_api(CommandBufferHandle command_buffer, std::span<const CommandPacket> commands) = 0;

        virtual void submit_api(const SubmitDesc& desc) = 0;
        virtual void present_api(const SwapchainPresentDesc& desc) = 0;

        virtual void wait_for_fence_api(FenceHandle fence) = 0;
        virtual void wait_queue_idle_api(CommandQueueType queue_type) = 0;
        virtual void wait_idle_api() = 0;

        virtual void bind_descriptor_api(const DescriptorBufferBinding& binding) = 0;

        virtual std::uint32_t acquire_next_image_api(SwapchainHandle swapchain, SemaphoreHandle semaphore, FenceHandle fence) = 0;
        virtual ImageHandle get_swapchain_image_api(SwapchainHandle swapchain, std::uint32_t image_index) = 0;

        spdlog::logger* logger_;
    };
} // namespace orion
