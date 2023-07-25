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
        device()->reset_command_pool(frame_data.transfer_command_pool);

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

        // Advance to next frame
        advance_frame_index();
    }

    void Renderer::present(SwapchainHandle swapchain)
    {
        // Get frame data
        auto& frame_data = previous_frame_data();

        // Get present semaphore
        const auto swapchain_image_semaphore = frame_data.swapchain_image_semaphore;

        // Acquire swapchain image
        const auto image_index = device()->acquire_next_image(swapchain, swapchain_image_semaphore, {});
        const auto swapchain_image = device()->get_swapchain_image(swapchain, image_index);

        // Get present command list
        auto& present_command = frame_data.present_command;

        present_command.begin({.usage = CommandBufferUsage::OneTimeSubmit});

        // Transition render output image to transfer src
        const auto render_output_barrier = ImageBarrierDesc{
            .src_access = ResourceAccess::ColorAttachmentWrite,
            .dst_access = ResourceAccess::TransferRead,
            .old_layout = ImageLayout::TransferSrc,
            .new_layout = ImageLayout::TransferSrc,
            .image = frame_data.image,
        };
        {
            auto* cmd_pipeline_barrier = present_command.add_command<CmdPipelineBarrier>({});
            cmd_pipeline_barrier->src_stages = PipelineStage::ColorAttachmentOutput;
            cmd_pipeline_barrier->dst_stages = PipelineStage::Transfer;
            cmd_pipeline_barrier->image_barriers = {&render_output_barrier, 1};
        }

        // Transition swapchain image to transfer dst
        const auto swapchain_transfer_barrier = ImageBarrierDesc{
            .src_access = {},
            .dst_access = ResourceAccess::TransferWrite,
            .old_layout = ImageLayout::Undefined,
            .new_layout = ImageLayout::TransferDst,
            .image = swapchain_image,
        };
        {
            auto* cmd_pipeline_barrier = present_command.add_command<CmdPipelineBarrier>({});
            cmd_pipeline_barrier->src_stages = PipelineStage::TopOfPipe;
            cmd_pipeline_barrier->dst_stages = PipelineStage::Transfer;
            cmd_pipeline_barrier->image_barriers = {&swapchain_transfer_barrier, 1};
        }

        // Blit render output image to swapchain image
        {
            auto* cmd_blit_image = present_command.add_command<CmdBlitImage>({});
            cmd_blit_image->src_image = frame_data.image;
            cmd_blit_image->src_layout = ImageLayout::TransferSrc;
            cmd_blit_image->src_size = render_size_;
            cmd_blit_image->dst_image = swapchain_image;
            cmd_blit_image->dst_layout = ImageLayout::TransferDst;
            cmd_blit_image->dst_size = render_size_;
        }

        // Transition swapchain image to present source
        const auto swapchain_present_barrier = ImageBarrierDesc{
            .src_access = ResourceAccess::TransferWrite,
            .dst_access = ResourceAccess::MemoryRead,
            .old_layout = ImageLayout::TransferDst,
            .new_layout = ImageLayout::PresentSrc,
            .image = swapchain_image,
        };
        {
            auto* cmd_pipeline_barrier = present_command.add_command<CmdPipelineBarrier>({});
            cmd_pipeline_barrier->src_stages = PipelineStage::Transfer;
            cmd_pipeline_barrier->dst_stages = PipelineStage::BottomOfPipe;
            cmd_pipeline_barrier->image_barriers = {&swapchain_present_barrier, 1};
        }

        present_command.end();

        // Submit command buffer
        const auto swapchain_copy_semaphore = frame_data.swapchain_copy_semaphore;
        const auto command_buffers = std::array{present_command.command_buffer()};
        const auto wait_semaphores = std::array{
            frame_data.render_semaphore,
            swapchain_image_semaphore,
        };
        const auto wait_stages = std::array{
            PipelineStage::Transfer,
            PipelineStage::Transfer,
        };
        device()->submit({
            .queue_type = CommandQueueType::Any,
            .command_buffers = command_buffers,
            .wait_semaphores = wait_semaphores,
            .wait_stages = wait_stages,
            .signal_semaphores = {&swapchain_copy_semaphore, 1},
            .fence = frame_data.swapchain_copy_fence,
        });

        // Present copied image
        device()->present({
            .swapchain = swapchain,
            .wait_semaphore = swapchain_copy_semaphore,
            .image_index = image_index,
        });

        device()->wait_for_fence(frame_data.swapchain_copy_fence);
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
            .queue_type = CommandQueueType::Graphics,
            .command_buffers = command_buffers,
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
            auto present_command_buffer = device->create_command_buffer({.command_pool = render_command_pool});

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

            return {
                .render_command_pool = render_command_pool,
                .transfer_command_pool = transfer_command_pool,
                .render_command = {device, render_command_buffer, render_command_size},
                .present_command = {device, present_command_buffer, present_command_size},
                .render_fence = device->create_fence(true),
                .render_semaphore = device->create_semaphore(),
                .swapchain_image_semaphore = device->create_semaphore(),
                .swapchain_copy_semaphore = device->create_semaphore(),
                .swapchain_copy_fence = device->create_fence(false),
                .image = image,
                .image_view = image_view,
                .render_target = device->create_framebuffer({
                    .render_pass = render_pass_,
                    .attachments = {&image_view, 1},
                    .size = render_size_,
                }),
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
