#pragma once

#include "orion-renderer/config.h"

#include "shader.h"
#include "sprite_renderer.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_device.h"

#include "orion-core/dyn_lib.h"
#include "orion-core/platform.h"

#include "orion-utils/static_vector.h"

#include <memory>

#include <spdlog/logger.h>

namespace orion
{
    const char* default_backend_module(Platform platform = k_current_platform);

    struct RendererDesc {
        const char* backend_module = default_backend_module();
        pfnSelectPhysicalDevice device_select_fn = nullptr;
        Vector2_u render_size;
    };

    // Forward declare
    class Scene;

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto* backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto* device() const noexcept { return render_device_.get(); }

        void begin();
        void end();
        void draw(const Scene& scene);
        void draw_test_triangle();

    private:
        struct FrameData {
            ImageHandle render_image;
            ImageViewHandle render_image_view;
            FramebufferHandle render_target;
            std::unique_ptr<CommandAllocator> command_allocator;
            std::unique_ptr<CommandList> command_list;
            GPUJobHandle render_job;
        };
        using FrameDataArr = static_vector<FrameData, frames_in_flight>;

        static spdlog::logger* logger();

        [[nodiscard]] FrameData& current_frame() noexcept { return frames_[current_frame_index_]; }
        [[nodiscard]] const FrameData& current_frame() const noexcept { return frames_[current_frame_index_]; }
        [[nodiscard]] FrameData& previous_frame() noexcept { return frames_[previous_frame_index_]; }
        [[nodiscard]] const FrameData& previous_frame() const noexcept { return frames_[previous_frame_index_]; }

        void advance_frame() noexcept;

        [[nodiscard]] std::unique_ptr<RenderBackend> create_render_backend() const;
        [[nodiscard]] std::unique_ptr<RenderDevice> create_render_device(pfnSelectPhysicalDevice device_select_fn) const;
        [[nodiscard]] RenderPassHandle create_render_pass() const;
        [[nodiscard]] PipelineLayoutHandle create_triangle_pipeline_layout() const;
        [[nodiscard]] PipelineHandle create_triangle_pipeline() const;
        [[nodiscard]] FrameDataArr create_frame_data() const;

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
        ShaderManager shader_manager_;

        Vector2_u render_size_;

        RenderPassHandle render_pass_;
        PipelineLayoutHandle triangle_pipeline_layout_;
        ShaderEffect triangle_shader_effect_;
        PipelineHandle triangle_pipeline_;

        FrameDataArr frames_;
        std::int8_t current_frame_index_ = 0;
        std::int8_t previous_frame_index_ = -1;
    };
} // namespace orion
