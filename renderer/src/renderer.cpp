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

#include "imgui_impl_orion.h"
#include <imgui.h>

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
        , clear_color_(desc.clear_color)
        , command_pool_(create_command_pool())
        , command_buffer_(create_command_buffer())
        , render_command_(device(), command_buffer_, render_command_size)
        , render_image_(create_render_image())
        , render_image_view_(create_render_image_view())
        , render_target_(create_render_target())
        , render_pass_(create_render_pass())
        , render_fence_(create_render_fence())
        , render_semaphore_(device()->create_semaphore())
        , present_command_buffer_(device()->create_command_buffer({.command_pool = command_pool_}))
        , present_command_({device(), present_command_buffer_, present_command_size})
        , swapchain_image_semaphore_(device()->create_semaphore())
        , swapchain_copy_semaphore_(device()->create_semaphore())
        , swapchain_copy_fence_(device()->create_fence(false))
        , descriptor_pool_(create_descriptor_pool())
    {
        SPDLOG_LOGGER_DEBUG(logger(), "Render backend {} initialized.", backend()->name());
        SPDLOG_LOGGER_DEBUG(logger(), "Renderer initialized.");
    }

    void Renderer::begin()
    {
        // Wait for frame to finish
        device()->wait_for_fence(render_fence_);

        // Reset command pool
        device()->reset_command_pool(command_pool_);

        // Begin command list recording
        render_command_.begin({});

        // Begin render pass
        {
            auto* cmd_begin_render_pass = render_command_.add_command<CmdBeginRenderPass>({});
            cmd_begin_render_pass->render_pass = render_pass_;
            cmd_begin_render_pass->framebuffer = render_target_;
            cmd_begin_render_pass->render_area = render_size_;
            cmd_begin_render_pass->clear_color = clear_color_;
        }
    }

    void Renderer::end()
    {
        // End render pass
        render_command_.add_command<CmdEndRenderPass>({});

        // End command list recording
        render_command_.end();

        // Submit render command
        const auto command_buffers = std::array{render_command_.command_buffer()};
        device()->submit({
            .queue_type = CommandQueueType::Graphics,
            .command_buffers = command_buffers,
            .wait_semaphores = {},
            .wait_stages = {},
            .signal_semaphores = {&render_semaphore_, 1},
            .fence = render_fence_,
        });
    }

    void Renderer::present(SwapchainHandle swapchain)
    {
        // Acquire swapchain image
        const auto image_index = device()->acquire_next_image(swapchain, swapchain_image_semaphore_, {});
        auto swapchain_image = device()->get_swapchain_image(swapchain, image_index);

        // Begin presentation commands
        present_command_.begin({});

        // Transition render result to transfer source
        {
            auto* cmd_pipeline_barrier = present_command_.add_command<CmdPipelineBarrier>({});
            cmd_pipeline_barrier->src_stages = PipelineStage::ColorAttachmentOutput;
            cmd_pipeline_barrier->dst_stages = PipelineStage::Transfer;
            cmd_pipeline_barrier->image_barrier = ImageBarrierDesc{
                .src_access = ResourceAccess::ColorAttachmentWrite,
                .dst_access = ResourceAccess::TransferRead,
                .old_layout = ImageLayout::TransferSrc,
                .new_layout = ImageLayout::TransferSrc,
                .image = render_image_,
            };
        }

        // Transition swapchain image to transfer dst
        {
            auto* cmd_pipeline_barrier = present_command_.add_command<CmdPipelineBarrier>({});
            cmd_pipeline_barrier->src_stages = PipelineStage::TopOfPipe;
            cmd_pipeline_barrier->dst_stages = PipelineStage::Transfer;
            cmd_pipeline_barrier->image_barrier = ImageBarrierDesc{
                .src_access = {},
                .dst_access = ResourceAccess::TransferWrite,
                .old_layout = ImageLayout::Undefined,
                .new_layout = ImageLayout::TransferDst,
                .image = swapchain_image,
            };
        }

        // Blit the image
        {
            auto* cmd_blit_image = present_command_.add_command<CmdBlitImage>({});
            cmd_blit_image->src_image = render_image_;
            cmd_blit_image->src_layout = ImageLayout::TransferSrc;
            cmd_blit_image->src_size = render_size_;
            cmd_blit_image->dst_image = swapchain_image;
            cmd_blit_image->dst_layout = ImageLayout::TransferDst;
            cmd_blit_image->dst_size = render_size_;
        }

        // Transition swapchain image to present source
        {
            auto* cmd_pipeline_barrier = present_command_.add_command<CmdPipelineBarrier>({});
            cmd_pipeline_barrier->src_stages = PipelineStage::Transfer;
            cmd_pipeline_barrier->dst_stages = PipelineStage::BottomOfPipe;
            cmd_pipeline_barrier->image_barrier = ImageBarrierDesc{
                .src_access = ResourceAccess::TransferWrite,
                .dst_access = ResourceAccess::MemoryRead,
                .old_layout = ImageLayout::TransferDst,
                .new_layout = ImageLayout::PresentSrc,
                .image = swapchain_image,
            };
        }

        // End presentation commands
        present_command_.end();

        // Submit commands
        const auto wait_semaphores = std::array{
            swapchain_image_semaphore_,
            render_semaphore_,
        };
        const auto wait_stages = std::array{
            PipelineStage::Transfer,
            PipelineStage::Transfer,
        };
        device()->submit({
            .queue_type = CommandQueueType::Any,
            .command_buffers = {&present_command_buffer_, 1},
            .wait_semaphores = wait_semaphores,
            .wait_stages = wait_stages,
            .signal_semaphores = {&swapchain_copy_semaphore_, 1},
            .fence = swapchain_copy_fence_,
        });

        // Wait until copy is done
        device()->wait_for_fence(swapchain_copy_fence_);

        // Present swapchain image
        device()->present({
            .swapchain = swapchain,
            .wait_semaphore = swapchain_copy_semaphore_,
            .image_index = image_index,
        });
    }

    void Renderer::imgui_init(Window* window)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        // Setup Platform/Renderer backends
        ImGui_ImplOrion_Init({
            .window = window,
            .device = device(),
            .descriptor_pool = descriptor_pool_,
        });
        SPDLOG_LOGGER_DEBUG(logger(), "ImGui initialized");
    }

    void Renderer::imgui_shutdown()
    {
        ImGui_ImplOrion_Shutdow();
        ImGui::DestroyContext();
        SPDLOG_LOGGER_DEBUG(logger(), "ImGui shut down");
    }

    void Renderer::imgui_new_frame()
    {
        ImGui_ImplOrion_NewFrame();
        ImGui::NewFrame();
    }

    void Renderer::imgui_render()
    {
        ImGui::Render();
        ImGui_ImplOrion_RenderDrawData(ImGui::GetDrawData(), render_command_);
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

    CommandPoolHandle Renderer::create_command_pool() const
    {
        return device()->create_command_pool({.queue_type = CommandQueueType::Graphics});
    }

    CommandBufferHandle Renderer::create_command_buffer() const
    {
        return device()->create_command_buffer({.command_pool = command_pool_});
    }

    FenceHandle Renderer::create_render_fence() const
    {
        return device()->create_fence(true);
    }

    ImageHandle Renderer::create_render_image() const
    {
        const auto desc = ImageDesc{
            .type = ImageType::Image2D,
            .format = Format::B8G8R8A8_Srgb,
            .size = {render_size_.x(), render_size_.y(), 1},
            .tiling = ImageTiling::Optimal,
            .usage = ImageUsageFlags::disjunction({ImageUsage::ColorAttachment, ImageUsage::TransferSrc}),
        };
        return device()->create_image(desc);
    }

    ImageViewHandle Renderer::create_render_image_view() const
    {
        const auto desc = ImageViewDesc{
            .image = render_image_,
            .type = ImageViewType::View2D,
            .format = Format::B8G8R8A8_Srgb,
        };
        return device()->create_image_view(desc);
    }

    RenderPassHandle Renderer::create_render_pass() const
    {
        const auto color_attachments = std::array{
            AttachmentDesc{
                .format = Format::B8G8R8A8_Srgb,
                .load_op = AttachmentLoadOp::Clear,
                .store_op = AttachmentStoreOp::Store,
                .initial_layout = ImageLayout::Undefined,
                .layout = ImageLayout::ColorAttachment,
                .final_layout = ImageLayout::TransferSrc,
            },
        };
        const auto desc = RenderPassDesc{
            .attachments = {
                .color_attachments = color_attachments,
            },
        };
        return device()->create_render_pass(desc);
    }

    FramebufferHandle Renderer::create_render_target() const
    {
        const auto attachment_list = std::array{
            AttachmentDesc{
                .format = Format::B8G8R8A8_Srgb,
            },
        };
        const auto image_views = std::array{render_image_view_};
        const auto desc = FramebufferDesc{
            .attachment_list = {attachment_list},
            .image_views = image_views,
            .size = render_size_,
        };
        return device()->create_framebuffer(desc);
    }

    DescriptorPoolHandle Renderer::create_descriptor_pool() const
    {
        const auto pool_sizes = std::array{
            DescriptorPoolSize{
                .type = DescriptorType::ConstantBuffer,
                .count = 16,
            },
            DescriptorPoolSize{
                .type = DescriptorType::SampledImage,
                .count = 16,
            },
            DescriptorPoolSize{
                .type = DescriptorType::ImageSampler,
                .count = 4,
            },
        };
        const auto desc = DescriptorPoolDesc{
            .max_sets = 16,
            .pool_sizes = pool_sizes,
        };
        return device()->create_descriptor_pool(desc);
    }
} // namespace orion
