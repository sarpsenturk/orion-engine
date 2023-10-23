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
        static constexpr auto frames_in_flight = std::int32_t{ORION_FRAMES_IN_FLIGHT};
        static_assert(frames_in_flight > 0);

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
            CommandList render_command;
            CommandPoolHandle command_pool;
            CommandBufferHandle command_buffer;
            FenceHandle render_fence;
            SemaphoreHandle render_semaphore;
        };
        using FrameDataArr = std::array<FrameData, frames_in_flight>;

        static constexpr auto render_command_size = 2048ull;

        static spdlog::logger* logger();

        [[nodiscard]] FrameData& current_frame() noexcept { return frames_[current_frame_index_]; }
        [[nodiscard]] const FrameData& current_frame() const noexcept { return frames_[current_frame_index_]; }
        [[nodiscard]] FrameData& previous_frame() noexcept { return frames_[previous_frame_index_]; }
        [[nodiscard]] const FrameData& previous_frame() const noexcept { return frames_[previous_frame_index_]; }

        void advance_frame() noexcept;

        std::unique_ptr<RenderBackend> create_render_backend() const;
        std::unique_ptr<RenderDevice> create_render_device(pfnSelectPhysicalDevice device_select_fn) const;
        FrameDataArr create_frame_data() const;

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        Vector2_u render_size_;

        ShaderManager shader_manager_;

        PipelineHandle present_pipeline_;
        RenderPassHandle present_render_pass_;

        std::array<FrameData, frames_in_flight> frames_;
        std::int32_t current_frame_index_ = 0;
        std::int32_t previous_frame_index_ = -1;
    };
} // namespace orion
