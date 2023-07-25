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

    struct FrameData {
        CommandPoolHandle render_command_pool;
        CommandPoolHandle transfer_command_pool;

        CommandList render_command;
        CommandList present_command;

        FenceHandle render_fence;
        SemaphoreHandle render_semaphore;
        SemaphoreHandle swapchain_image_semaphore;
        SemaphoreHandle swapchain_copy_semaphore;

        ImageHandle image;
        ImageViewHandle image_view;
        FramebufferHandle render_target;
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
        void present(SwapchainHandle swapchain);

        void advance_frame_index() noexcept
        {
            previous_frame_ = current_frame_;
            current_frame_ = (current_frame_ + 1) % frames_in_flight;
        }
        [[nodiscard]] auto current_frame_index() const noexcept { return current_frame_; }
        [[nodiscard]] auto previous_frame_index() const noexcept { return previous_frame_; }
        [[nodiscard]] auto& current_frame_data() noexcept { return frame_data_[current_frame_index()]; }
        [[nodiscard]] auto& current_frame_data() const noexcept { return frame_data_[current_frame_index()]; }
        [[nodiscard]] auto& previous_frame_data() noexcept { return frame_data_[previous_frame_index()]; }
        [[nodiscard]] auto& previous_frame_data() const noexcept { return frame_data_[previous_frame_index()]; }

        static spdlog::logger* logger();

    private:
        static constexpr auto render_command_size = 2048;
        static constexpr auto present_command_size = 256;

        void submit_frame(const FrameData& frame_data) const;

        std::unique_ptr<RenderBackend> create_backend(const Module& backend_module) const;
        std::unique_ptr<RenderDevice> create_device(RenderBackend* backend, pfnSelectPhysicalDevice device_select_fn) const;
        RenderPassHandle create_render_pass(RenderDevice* device) const;
        static_vector<FrameData, frames_in_flight> create_frame_data(RenderDevice* device) const;

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        Vector2_u render_size_;
        Vector4_f color_clear_ = {1.f, 0.f, 1.f, 1.f};

        RenderPassHandle render_pass_;

        static_vector<FrameData, frames_in_flight> frame_data_;
        std::uint8_t current_frame_ = 0;
        std::uint8_t previous_frame_ = 0;
    };
} // namespace orion
