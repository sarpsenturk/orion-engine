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
    }

    void Renderer::end_frame()
    {
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

    static_vector<Renderer::FrameData, Renderer::frames_in_flight> Renderer::create_frame_data(RenderDevice* device) const
    {
        auto create_data = [device, this]() -> FrameData {
            auto render_command_pool = device->create_command_pool({.queue_type = CommandQueueType::Graphics});
            auto render_command_buffer = device->create_command_buffer({.command_pool = render_command_pool});
            auto render_fence = device->create_fence(true);
            auto color_image = device->create_image({
                .type = ImageType::Image2D,
                .format = Format::B8G8R8A8_Srgb,
                .size = {render_size_.x(), render_size_.y(), 1},
                .tiling = ImageTiling::Optimal,
                .usage = ImageUsageFlags::disjunction({ImageUsage::ColorAttachment, ImageUsage::TransferSrc}),
            });
            auto color_image_view = device->create_image_view({
                .image = color_image,
                .type = ImageViewType::View2D,
                .format = Format::B8G8R8A8_Srgb,
            });

            auto color_render_target = device->create_framebuffer({
                .render_pass = render_pass_,
                .attachments = {&color_image_view, 1},
                .size = render_size_,
            });
            return {
                .render_command_pool = render_command_pool,
                .render_command = {device, render_command_buffer, render_command_size},
                .render_fence = render_fence,
                .color_image = color_image,
                .color_image_view = color_image_view,
                .color_render_target = color_render_target,
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
