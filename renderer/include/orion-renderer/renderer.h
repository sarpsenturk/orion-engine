#pragma once

#include "orion-renderer/config.h"
#include "orion-renderer/frame.h"
#include "orion-renderer/imgui.h"
#include "orion-renderer/quad_renderer.h"
#include "orion-renderer/render_window.h"
#include "orion-renderer/shader.h"
#include "orion-renderer/texture.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_device.h"

#include "orion-core/dyn_lib.h"
#include "orion-core/platform.h"

#include "orion-utils/static_vector.h"

#include <memory>
#include <vector>

namespace spdlog
{
    class logger;
}

namespace orion
{
    const char* default_backend_module(Platform platform = current_platform);

    struct RendererDesc {
        const char* backend_module = default_backend_module();
        SelectPhysicalDeviceFn device_select_fn = nullptr;
        Vector2_u render_size;
    };

    // Forward declare
    class Scene;
    class Window;

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);
        Renderer(const Renderer&) = delete;
        Renderer(Renderer&&) noexcept = default;
        Renderer& operator=(const Renderer&) = delete;
        Renderer& operator=(Renderer&&) noexcept = default;
        ~Renderer();

        [[nodiscard]] auto* backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto* device() const noexcept { return render_device_.get(); }

        [[nodiscard]] auto& shader_manager() { return shader_manager_; }
        [[nodiscard]] auto& texture_manager() { return texture_manager_; }

        [[nodiscard]] QuadRenderer create_quad_renderer();

        void begin();
        void end();
        void render(QuadRenderer& quad_renderer);

        RenderWindow create_render_window(Window& window, bool vsync);
        void present(RenderWindow& render_window);

        ImGuiContext imgui_init(Window& window);
        void imgui_new_frame();
        void imgui_render();

    private:
        struct FrameData {
            ImageHandle render_image;
            ImageViewHandle render_image_view;
            FramebufferHandle render_target;
            std::unique_ptr<CommandList> command_list;
            FenceHandle fence;
            SemaphoreHandle render_semaphore;
        };

        static spdlog::logger* logger();

        [[nodiscard]] std::unique_ptr<RenderBackend> create_render_backend() const;
        [[nodiscard]] std::unique_ptr<RenderDevice> create_render_device(SelectPhysicalDeviceFn device_select_fn) const;
        [[nodiscard]] std::unique_ptr<CommandAllocator> create_command_allocator() const;
        [[nodiscard]] RenderPassHandle create_render_pass() const;
        [[nodiscard]] FrameData create_frame_data() const;

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        RenderPassHandle render_pass_;

        std::unique_ptr<CommandAllocator> command_allocator_;

        ShaderManager shader_manager_;
        TextureManager texture_manager_;

        Vector2_u render_size_;
        Viewport viewport_;
        Scissor scissor_;

        PerFrameData<FrameData> frames_;
        frame_index_t current_frame_index_ = 0;
        frame_index_t previous_frame_index_ = -1;
        void advance_frame();

        RenderContext current_context() const;
    };
} // namespace orion
