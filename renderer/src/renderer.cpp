#include "orion-renderer/renderer.h"

#include "orion-core/window.h"
#include "orion-renderer/mesh.h"
#include "orion-utils/assertion.h"

#include <array>

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
        , render_size_(desc.render_size)
        , render_pass_(create_render_pass(device()))
        , frame_data_(create_frame_data(device()))
    {
        SPDLOG_LOGGER_DEBUG(logger(), "Render backend {} initialized.", backend()->name());
        SPDLOG_LOGGER_DEBUG(logger(), "Renderer initialized.");
    }

    void Renderer::begin_frame()
    {
        // Get current frame data
        auto& frame_data = current_frame_data();

        // Wait for current frame to finish previous work
        device()->wait_for_fence(frame_data.render_fence);

        // Reset command list
        device()->reset_command_pool(frame_data.render_command_pool);

        // Get render command
        auto& render_command = frame_data.render_command;

        // Begin command list recording
        render_command.begin({});

        // Begin frame
        auto* cmd_begin_frame = render_command.add_command<CmdBeginFrame>({});
        cmd_begin_frame->render_pass = render_pass_;
        cmd_begin_frame->framebuffer = frame_data.render_target;
        cmd_begin_frame->render_area = render_size_;
        cmd_begin_frame->clear_color = color_clear_;
    }

    void Renderer::end_frame()
    {
        // Get current frame data
        auto& frame_data = current_frame_data();

        // Get render command
        auto& render_command = frame_data.render_command;

        // End frame
        render_command.add_command<CmdEndFrame>({});

        // End command list
        render_command.end();

        // Submit frame
        submit_frame(frame_data);
    }

    void Renderer::present(SwapchainHandle swapchain)
    {
    }

    spdlog::logger* Renderer::logger()
    {
        static const auto renderer_logger = create_logger("orion-renderer", static_cast<spdlog::level::level_enum>(ORION_RENDERER_LOG_LEVEL));
        return renderer_logger.get();
    }

    void Renderer::submit_frame(const FrameData& frame_data) const
    {
        const auto command_buffers = std::array{
            frame_data.render_command.command_buffer(),
        };
        const auto signal_semaphores = std::array{
            frame_data.render_semaphore,
        };
        const auto desc = SubmitDesc{
            .command_buffers = command_buffers,
            .queue_type = CommandQueueType::Graphics,
            .wait_semaphores = {},
            .signal_semaphores = signal_semaphores,
            .fence = frame_data.render_fence,
        };
        device()->submit(desc);
    }

    std::unique_ptr<RenderBackend> Renderer::create_backend(const Module& backend_module) const
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

    std::unique_ptr<RenderDevice> Renderer::create_device(RenderBackend* backend, pfnSelectPhysicalDevice device_select_fn) const
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

    RenderPassHandle Renderer::create_render_pass(RenderDevice* device) const
    {
        const auto color_attachments = std::array{
            RenderPassAttachmentDesc{
                .format = Format::B8G8R8A8_Srgb,
                .load_op = AttachmentLoadOp::Clear,
                .store_op = AttachmentStoreOp::Store,
                .initial_layout = ImageLayout::Undefined,
                .layout = ImageLayout::ColorAttachment,
                .final_layout = ImageLayout::TransferSrc,
            },
        };
        const auto desc = RenderPassDesc{
            .color_attachments = color_attachments,
        };
        return device->create_render_pass(desc);
    }

    static_vector<FrameData, Renderer::frames_in_flight> Renderer::create_frame_data(RenderDevice* device) const
    {
        auto create_data = [device, this]() -> FrameData {
            // Create command pools
            auto render_command_pool = device->create_command_pool({.queue_type = CommandQueueType::Graphics});
            auto transfer_command_pool = device->create_command_pool({.queue_type = CommandQueueType::Transfer});

            // Create command buffers
            auto render_command_buffer = device->create_command_buffer({.command_pool = render_command_pool});
            auto present_command_buffer = device->create_command_buffer({.command_pool = transfer_command_pool});

            // Create sync objects
            auto render_fence = device->create_fence(true);
            auto render_semaphore = device->create_semaphore();
            auto swapchain_copy_semaphore = device->create_semaphore();
            auto present_semaphore = device->create_semaphore();

            // Create render images
            auto image = device->create_image({
                .type = ImageType::Image2D,
                .format = Format::B8G8R8A8_Srgb,
                .size = {render_size_.x(), render_size_.y(), 1},
                .tiling = ImageTiling::Optimal,
                .usage = ImageUsageFlags::disjunction({ImageUsage::ColorAttachment, ImageUsage::TransferSrc}),
            });
            auto image_view = device->create_image_view({
                .image = image,
                .type = ImageViewType::View2D,
                .format = Format::B8G8R8A8_Srgb,
            });
            auto render_target = device->create_framebuffer({
                .render_pass = render_pass_,
                .attachments = {&image_view, 1},
                .size = render_size_,
            });

            return {
                .render_command_pool = render_command_pool,
                .transfer_command_pool = transfer_command_pool,
                .render_command = {device, render_command_buffer, render_command_size},
                .render_fence = render_fence,
                .render_semaphore = render_semaphore,
                .present_semaphore = present_semaphore,
                .image = image,
                .image_view = image_view,
                .render_target = render_target,
            };
        };

        SPDLOG_LOGGER_TRACE(logger(), "Creating FrameData for {} frames in flight...", frames_in_flight);
        static_vector<FrameData, frames_in_flight> frame_data;
        for (int i = 0; i < frames_in_flight; ++i) {
            frame_data.push_back(create_data());
        }
        return frame_data;
    }
} // namespace orion
