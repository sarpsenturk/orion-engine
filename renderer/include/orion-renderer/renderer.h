#pragma once

#include "orion-renderer/config.h"
#include "shader_compiler.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_device.h"

#include "orion-core/config.h"
#include "orion-core/platform.h"

#include "orion-utils/static_vector.h"

#include <memory>
#include <span>
#include <spdlog/logger.h>

namespace orion
{
    const char* default_backend_module(Platform platform = current_platform);

    struct RendererDesc {
        const char* backend_module = default_backend_module();
        pfnSelectPhysicalDevice device_select_fn = nullptr;
        Vector2_u render_size = {};
    };

    class Renderer
    {
    public:
        static constexpr auto frames_in_flight = ORION_FRAMES_IN_FLIGHT;

        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto device() const noexcept { return render_device_.get(); }

        void begin_frame();
        void end_frame();
        void present(const SwapchainPresentDesc& desc);

        [[nodiscard]] auto frame_index() const noexcept { return current_frame_; }

        static spdlog::logger* logger();

    private:
        struct FrameData {
            CommandPoolHandle render_command_pool;
            CommandList render_command;
            FenceHandle render_fence;

            ImageHandle color_image;
            ImageViewHandle color_image_view;
        };

        static constexpr auto render_command_size = 2048;

        std::unique_ptr<RenderBackend> create_backend(const Module& backend_module) const;
        std::unique_ptr<RenderDevice> create_device(RenderBackend* backend, pfnSelectPhysicalDevice device_select_fn) const;
        static_vector<FrameData, frames_in_flight> create_frame_data(RenderDevice* device) const;

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
        Vector2_u render_size_;

        static_vector<FrameData, frames_in_flight> frame_data_;
        std::uint32_t current_frame_ = 0;
    };
} // namespace orion
