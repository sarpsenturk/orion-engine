#pragma once

#include "orion-renderer/config.h"
#include "shader_compiler.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_device.h"

#include "orion-core/platform.h"
#include "orion-core/window.h"

#include <memory>
#include <span>
#include <spdlog/logger.h>

namespace orion
{
    const char* default_backend_module(Platform platform = k_current_platform);

    struct RendererDesc {
        const char* backend_module = default_backend_module();
        pfnSelectPhysicalDevice device_select_fn = nullptr;
        Vector2_u render_size;
        Vector4_f clear_color;
    };

    class Renderer
    {
    public:
        static constexpr auto frames_in_flight = ORION_FRAMES_IN_FLIGHT;

        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto device() const noexcept { return render_device_.get(); }

        void begin();
        void end();
        void present(SwapchainHandle swapchain);

        void resize_images(const Vector2_u& new_size);

        void imgui_init(Window* window);
        void imgui_shutdown();

        void imgui_new_frame();
        void imgui_render();

        static spdlog::logger* logger();

    private:
        static constexpr auto render_command_size = 2048ull;
        static constexpr auto present_command_size = 256ull;

        [[nodiscard]] std::unique_ptr<RenderBackend> create_backend(const Module& backend_module) const;
        [[nodiscard]] std::unique_ptr<RenderDevice> create_device(RenderBackend* backend, pfnSelectPhysicalDevice device_select_fn) const;
        [[nodiscard]] Viewport create_viewport() const;
        [[nodiscard]] Scissor create_scissor() const;
        [[nodiscard]] CommandPoolHandle create_command_pool() const;
        [[nodiscard]] CommandBufferHandle create_command_buffer() const;
        [[nodiscard]] FenceHandle create_render_fence() const;
        [[nodiscard]] ImageHandle create_render_image() const;
        [[nodiscard]] ImageViewHandle create_render_image_view() const;
        [[nodiscard]] RenderPassHandle create_render_pass() const;
        [[nodiscard]] FramebufferHandle create_render_target() const;
        [[nodiscard]] DescriptorPoolHandle create_descriptor_pool() const;

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        Vector2_u render_size_;
        Vector4_f clear_color_;

        Viewport viewport_;
        Scissor scissor_;

        CommandPoolHandle command_pool_;
        CommandBufferHandle command_buffer_;
        CommandList render_command_;

        ImageHandle render_image_;
        ImageViewHandle render_image_view_;
        FramebufferHandle render_target_;

        RenderPassHandle render_pass_;

        FenceHandle render_fence_;
        SemaphoreHandle render_semaphore_;

        CommandBufferHandle present_command_buffer_;
        CommandList present_command_;
        SemaphoreHandle swapchain_image_semaphore_;
        SemaphoreHandle swapchain_copy_semaphore_;
        FenceHandle swapchain_copy_fence_;

        DescriptorPoolHandle descriptor_pool_;
    };
} // namespace orion
