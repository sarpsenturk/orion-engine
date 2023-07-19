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
    };

    struct FrameBeginDesc {
        RenderPassHandle render_pass;
        FramebufferHandle framebuffer;
        Vector2_u render_area = {};
        Vector4_f clear_color = {};
    };

    struct FrameEndDesc {
        std::span<const SemaphoreHandle> wait_semaphores;
        std::span<const SemaphoreHandle> signal_semaphores;
    };

    class Renderer
    {
    public:
        static constexpr auto frames_in_flight = ORION_FRAMES_IN_FLIGHT;

        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto device() const noexcept { return render_device_.get(); }

        void begin_frame(const FrameBeginDesc& desc);
        void end_frame(const FrameEndDesc& desc);
        void present(const SwapchainPresentDesc& desc);

        [[nodiscard]] auto frame_index() const noexcept { return current_frame_; }

        static spdlog::logger* logger();

    private:
        struct FrameData {
            CommandPoolHandle render_command_pool;
            CommandList render_command;
            FenceHandle render_fence;
        };

        static constexpr auto render_command_size = 2048;

        static std::unique_ptr<RenderBackend> create_backend(const Module& backend_module);
        static std::unique_ptr<RenderDevice> create_device(RenderBackend* backend, pfnSelectPhysicalDevice device_select_fn);
        static static_vector<FrameData, frames_in_flight> create_frame_data(RenderDevice* device);

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        static_vector<FrameData, frames_in_flight> frame_data_;
        std::uint32_t current_frame_ = 0;
    };
} // namespace orion
