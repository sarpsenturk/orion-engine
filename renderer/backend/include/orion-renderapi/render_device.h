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

#include <atomic>
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

    class DeviceResourceControlBlock
    {
    public:
        explicit DeviceResourceControlBlock(RenderDevice* device);

        std::size_t add_ref();
        std::size_t remove_ref();

        [[nodiscard]] auto device() const noexcept { return device_; }

    private:
        RenderDevice* device_ = nullptr;
        std::atomic_size_t ref_count_ = 1;
    };

    template<typename HandleType>
    class DeviceResource
    {
    public:
        using handle_type = HandleType;

        DeviceResource() = default;
        DeviceResource(handle_type handle, RenderDevice* device)
            : handle_(handle)
            , control_block_(new DeviceResourceControlBlock(device))
        {
        }
        DeviceResource(const DeviceResource& other)
            : handle_(other.handle_)
            , control_block_(other.control_block_)
        {
            control_block_->add_ref();
        }
        DeviceResource(DeviceResource&& other) noexcept
            : handle_(std::exchange(other.handle_, handle_type::invalid_handle()))
            , control_block_(other.control_block_)
        {
        }
        DeviceResource& operator=(const DeviceResource& other)
        {
            if ((&other) == this) {
                return *this;
            }
            handle_ = other.handle_;
            control_block_ = other.control_block_;
            control_block_->add_ref();
            return *this;
        }
        DeviceResource& operator=(DeviceResource&& other) noexcept
        {
            if ((&other) == this) {
                return *this;
            }
            handle_ = std::exchange(other.handle_, handle_type::invalid_handle());
            control_block_ = other.control_block_;
            return *this;
        }
        ~DeviceResource()
        {
            if (control_block_->remove_ref() == 0) {
                control_block_->device()->destroy(handle_);
                delete control_block_;
            }
        }

        [[nodiscard]] HandleType get() const noexcept { return handle_; }

    private:
        handle_type handle_;
        DeviceResourceControlBlock* control_block_ = nullptr;
    };

    using SwapchainResource = DeviceResource<SwapchainHandle>;
    using RenderPassResource = DeviceResource<RenderPassHandle>;
    using ShaderResource = DeviceResource<ShaderModuleHandle>;
    using PipelineResource = DeviceResource<PipelineHandle>;
    using GPUBufferResource = DeviceResource<GPUBufferHandle>;
    using CommandPoolResource = DeviceResource<CommandPoolHandle>;
    using CommandBufferResource = DeviceResource<CommandBufferHandle>;
    using DescriptorPoolResource = DeviceResource<DescriptorPoolHandle>;
    using DescriptorSetResource = DeviceResource<DescriptorSetHandle>;

    class RenderDevice
    {
    public:
        explicit RenderDevice(spdlog::logger* logger);
        virtual ~RenderDevice() = default;

        template<typename Handle>
        auto make_resource(Handle handle)
        {
            return DeviceResource(handle, this);
        }

        [[nodiscard]] SwapchainHandle create_swapchain(const Window& window, const SwapchainDesc& desc);
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

        void recreate(SwapchainHandle swapchain_handle, const SwapchainDesc& desc);
        void recreate(ImageHandle image_handle, const ImageDesc& desc);
        void recreate(ImageViewHandle image_view_handle, const ImageViewDesc& desc);
        void recreate(FramebufferHandle framebuffer_handle, const FramebufferDesc& desc);

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

        void update_descriptors(const DescriptorUpdate& update);

        std::uint32_t acquire_next_image(SwapchainHandle swapchain, SemaphoreHandle semaphore, FenceHandle fence);
        ImageHandle get_swapchain_image(SwapchainHandle swapchain, std::uint32_t image_index);

        [[nodiscard]] auto logger() const noexcept { return logger_; }

    protected:
        RenderDevice(const RenderDevice&) = default;
        RenderDevice(RenderDevice&&) noexcept = default;
        RenderDevice& operator=(const RenderDevice&) = default;
        RenderDevice& operator=(RenderDevice&&) noexcept = default;

    private:
        [[nodiscard]] virtual SwapchainHandle create_swapchain_api(const Window& window, const SwapchainDesc& desc) = 0;
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

        virtual void recreate_api(SwapchainHandle swapchain_handle, const SwapchainDesc& desc) = 0;
        virtual void recreate_api(ImageHandle image_handle, const ImageDesc& desc) = 0;
        virtual void recreate_api(ImageViewHandle image_view_handle, const ImageViewDesc& desc) = 0;
        virtual void recreate_api(FramebufferHandle framebuffer_handle, const FramebufferDesc& desc) = 0;

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

        virtual void update_descriptors_api(const DescriptorUpdate& update) = 0;

        virtual std::uint32_t acquire_next_image_api(SwapchainHandle swapchain, SemaphoreHandle semaphore, FenceHandle fence) = 0;
        virtual ImageHandle get_swapchain_image_api(SwapchainHandle swapchain, std::uint32_t image_index) = 0;

        spdlog::logger* logger_;
    };
} // namespace orion
