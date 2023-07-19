#include "orion-renderer/renderer.h"

#include "orion-core/window.h"
#include "orion-renderer/mesh.h"
#include "orion-utils/assertion.h"

#include <algorithm>
#include <array>
#include <ranges>

#ifndef ORION_RENDERER_LOG_LEVEL
    #define ORION_RENDERER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    const char* default_backend_module(Platform platform)
    {
        switch (platform) {
            case Platform::Unknown:
                break;
            case Platform::Windows:
                return ORION_VULKAN_MODULE;
            case Platform::Linux:
                break;
        }
        ORION_ASSERT(!"Current platform is not supported by the renderer");
        return nullptr;
    }

    Renderer::Renderer(const RendererDesc& desc)
        : backend_module_(desc.backend_module)
        , render_backend_(create_backend(backend_module_))
        , render_device_(create_device(backend(), desc.device_select_fn))
        , frame_data_(create_frame_data(device()))
    {
        SPDLOG_LOGGER_DEBUG(logger(), "Render backend {} initialized.", backend()->name());
        SPDLOG_LOGGER_DEBUG(logger(), "Renderer initialized.");
    }

    void Renderer::begin_frame(const FrameBeginDesc& desc)
    {
        // Get current frame data
        auto& frame_data = frame_data_[current_frame_];

        // Wait for gpu to finish current frame
        device()->wait_for_fence(frame_data.render_fence);

        // Get and clear command list
        auto& render_command = frame_data.render_command;
        render_command.clear();

        // Begin command list recording
        render_command.begin({});

        // Begin frame
        auto* cmd_begin_frame = render_command.add_command<CmdBeginFrame>({});
        cmd_begin_frame->render_pass = desc.render_pass;
        cmd_begin_frame->framebuffer = desc.framebuffer;
        cmd_begin_frame->render_area = desc.render_area;
        cmd_begin_frame->clear_color = desc.clear_color;
    }

    void Renderer::end_frame(const FrameEndDesc& desc)
    {
        // Get frame data
        auto& frame_data = frame_data_[current_frame_];

        // Get command list
        auto& render_command = frame_data.render_command;

        // End frame
        render_command.add_command<CmdEndFrame>({});

        // End command list
        render_command.end();

        // Submit command buffer
        const auto command_buffers = std::array{
            render_command.command_buffer(),
        };
        const auto submission = SubmitDesc{
            .command_buffers = command_buffers,
            .queue_type = CommandQueueType::Graphics,
            .wait_semaphores = desc.wait_semaphores,
            .signal_semaphores = desc.signal_semaphores,
            .fence = frame_data.render_fence,
        };
        device()->submit(submission);
    }

    void Renderer::present(const SwapchainPresentDesc& desc)
    {
        device()->present(desc);
    }

    spdlog::logger* Renderer::logger()
    {
        static const auto renderer_logger = create_logger("orion-renderer", static_cast<spdlog::level::level_enum>(ORION_RENDERER_LOG_LEVEL));
        return renderer_logger.get();
    }

    std::unique_ptr<RenderBackend> Renderer::create_backend(const Module& backend_module)
    {
        SPDLOG_LOGGER_TRACE(logger(), "Initializing render backend...");

        // Load create_render_backend function
        auto* create_backend_fn = backend_module.load_symbol<pfnCreateRenderBackend>("create_render_backend");

        // Create render backend
        auto render_backend = std::unique_ptr<RenderBackend>(create_backend_fn());
        if (!render_backend) {
            throw std::runtime_error("Failed to initialize render backend");
        }
        return render_backend;
    }

    std::unique_ptr<RenderDevice> Renderer::create_device(RenderBackend* backend, pfnSelectPhysicalDevice device_select_fn)
    {
        SPDLOG_LOGGER_TRACE(logger(), "Creating render device...");

        ORION_ASSERT(backend != nullptr);
        // Get the physical devices
        auto physical_devices = backend->enumerate_physical_devices();
        SPDLOG_LOGGER_DEBUG(logger(), "Found {} physical device(s):", physical_devices.size());
        for (const auto& physical_device : physical_devices) {
            SPDLOG_LOGGER_DEBUG(logger(), "{}", physical_device.name);
            SPDLOG_LOGGER_DEBUG(logger(), "-- Index: {}", physical_device.index);
            SPDLOG_LOGGER_DEBUG(logger(), "-- Type: {}", physical_device.type);
        }

        // Select the physical device to use
        const auto physical_device_index =
            [device_select_fn, &physical_devices]() -> uint32_t {
            if (device_select_fn != nullptr) {
                return device_select_fn(physical_devices);
            }
            return 0;
        }();
        if (physical_device_index == UINT32_MAX) {
            throw std::runtime_error("Couldn't find a suitable physical device");
        }

        SPDLOG_LOGGER_INFO(logger(), "Using physical device index {}", physical_device_index);

        // Create device
        return backend->create_device(physical_device_index);
    }

    static_vector<Renderer::FrameData, Renderer::frames_in_flight> Renderer::create_frame_data(RenderDevice* device)
    {
        auto create_data = [device]() -> FrameData {
            auto render_command_pool = device->create_command_pool({.queue_type = CommandQueueType::Graphics});
            auto render_command_buffer = device->create_command_buffer({.command_pool = render_command_pool});
            auto render_fence = device->create_fence(true);
            return {
                .render_command_pool = render_command_pool,
                .render_command = {device, render_command_buffer, render_command_size},
                .render_fence = render_fence,
            };
        };

        static_vector<FrameData, frames_in_flight> frame_data;
        for (int i = 0; i < frames_in_flight; ++i) {
            frame_data.push_back(create_data());
        }
        return frame_data;
    }
} // namespace orion
