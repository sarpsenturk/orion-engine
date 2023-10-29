#pragma once

#include "orion-renderer/config.h"

#include "shader.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_device.h"

#include "orion-core/dyn_lib.h"
#include "orion-core/platform.h"

#include <array>
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

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto* backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto* device() const noexcept { return render_device_.get(); }

        void begin();
        void end();

    private:
        struct FrameData {
            ImageHandle render_image;
            ImageViewHandle render_image_view;
            FramebufferHandle render_target;
            CommandPoolHandle command_pool;
            GPUJobHandle frame_job;
        };
        using FrameDataArr = std::array<FrameData, frames_in_flight>;

        static spdlog::logger* logger();

        [[nodiscard]] FrameData& current_frame() noexcept { return frames_[current_frame_index_]; }
        [[nodiscard]] const FrameData& current_frame() const noexcept { return frames_[current_frame_index_]; }
        [[nodiscard]] FrameData& previous_frame() noexcept { return frames_[previous_frame_index_]; }
        [[nodiscard]] const FrameData& previous_frame() const noexcept { return frames_[previous_frame_index_]; }

        void advance_frame() noexcept;

        [[nodiscard]] std::unique_ptr<RenderBackend> create_render_backend() const;
        [[nodiscard]] std::unique_ptr<RenderDevice> create_render_device(pfnSelectPhysicalDevice device_select_fn) const;
        [[nodiscard]] FrameDataArr create_frame_data() const;

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        Vector2_u render_size_;

        ShaderManager shader_manager_;

        PipelineHandle present_pipeline_;
        RenderPassHandle present_render_pass_;

        FrameDataArr frames_;
        std::int32_t current_frame_index_ = 0;
        std::int32_t previous_frame_index_ = -1;
    };
} // namespace orion
