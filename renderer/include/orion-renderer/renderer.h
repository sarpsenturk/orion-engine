#pragma once

#include "orion-renderer/config.h"

#include "shader.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_device.h"

#include "orion-core/dyn_lib.h"
#include "orion-core/platform.h"

#include "orion-utils/static_vector.h"

#include <memory>
#include <vector>

#include <spdlog/logger.h>

namespace orion
{
    const char* default_backend_module(Platform platform = current_platform);

    struct RendererDesc {
        Window* window;
        const char* backend_module = default_backend_module();
        SelectPhysicalDeviceFn device_select_fn = nullptr;
        Vector2_u render_size;
        bool init_imgui;
    };

    // Forward declare
    class Scene;
    class Window;

    class ImGuiContext
    {
    public:
        ~ImGuiContext();
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto* backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto* device() const noexcept { return render_device_.get(); }

        void imgui_init();

        void begin();
        void end();
        void imgui_begin();
        void imgui_end();
        void draw(const Scene& scene);
        void draw_test_triangle();

        void present(const Window& window);

    private:
        struct FrameData {
            ImageHandle render_image;
            ImageViewHandle render_image_view;
            FramebufferHandle render_target;
            std::unique_ptr<CommandList> command_list;
            std::unique_ptr<CommandList> present_command;
            FenceHandle fence;
            SemaphoreHandle render_semaphore;
            SemaphoreHandle present_semaphore;
            FenceHandle present_fence;
            DescriptorHandle present_descriptor;
        };
        using FrameDataArr = std::vector<FrameData>;

        static constexpr auto swapchain_image_count = frames_in_flight;
        static constexpr auto swapchain_image_format = Format::B8G8R8A8_Srgb;

        static spdlog::logger* logger();

        [[nodiscard]] FrameData& current_frame() noexcept { return frames_[current_frame_index_]; }
        [[nodiscard]] const FrameData& current_frame() const noexcept { return frames_[current_frame_index_]; }
        [[nodiscard]] FrameData& previous_frame() noexcept { return frames_[previous_frame_index_]; }
        [[nodiscard]] const FrameData& previous_frame() const noexcept { return frames_[previous_frame_index_]; }

        void advance_frame() noexcept;

        [[nodiscard]] std::unique_ptr<RenderBackend> create_render_backend() const;
        [[nodiscard]] std::unique_ptr<RenderDevice> create_render_device(SelectPhysicalDeviceFn device_select_fn) const;
        [[nodiscard]] std::unique_ptr<Swapchain> create_swapchain(const Window* window) const;
        [[nodiscard]] std::vector<ImageViewHandle> create_swapchain_image_views() const;
        [[nodiscard]] std::vector<FramebufferHandle> create_swapchain_framebuffers(const Vector2_u& size) const;
        [[nodiscard]] std::unique_ptr<CommandAllocator> create_command_allocator() const;
        [[nodiscard]] RenderPassHandle create_render_pass() const;
        [[nodiscard]] PipelineLayoutHandle create_triangle_pipeline_layout() const;
        [[nodiscard]] PipelineHandle create_triangle_pipeline() const;
        [[nodiscard]] DescriptorLayoutHandle create_present_descriptor_layout() const;
        [[nodiscard]] PipelineLayoutHandle create_present_pipeline_layout() const;
        [[nodiscard]] SamplerHandle create_present_sampler() const;
        [[nodiscard]] RenderPassHandle create_present_pass() const;
        [[nodiscard]] PipelineHandle create_present_pipeline() const;
        [[nodiscard]] FrameDataArr create_frame_data() const;
        [[nodiscard]] std::unique_ptr<ImGuiContext> create_imgui_context(Window* window);

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        RenderPassHandle render_pass_;
        RenderPassHandle present_pass_;

        std::unique_ptr<Swapchain> swapchain_;
        std::vector<ImageViewHandle> swapchain_image_views_;
        std::vector<FramebufferHandle> swapchain_framebuffers_;

        std::unique_ptr<CommandAllocator> command_allocator_;

        ShaderManager shader_manager_;

        Vector2_u render_size_;

        PipelineLayoutHandle triangle_pipeline_layout_;
        ShaderEffect triangle_shader_effect_;
        PipelineHandle triangle_pipeline_;

        DescriptorLayoutHandle present_descriptor_layout_;
        PipelineLayoutHandle present_pipeline_layout_;
        SamplerHandle present_sampler_;
        ShaderEffect present_shader_effect_;
        PipelineHandle present_pipeline_;

        FrameDataArr frames_;
        std::int8_t current_frame_index_ = 0;
        std::int8_t previous_frame_index_ = -1;

        std::unique_ptr<ImGuiContext> imgui_;
    };
} // namespace orion
