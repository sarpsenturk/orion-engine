#include "orion-renderer/renderer.h"

#include "orion-renderer/colors.h"

#include "imgui_impl_orion.h"
#include <imgui.h>

#include "orion-renderapi/config.h"

#include "orion-core/window.h"

#include "orion-utils/assertion.h"

#include "orion-scene/components.h"
#include "orion-scene/scene.h"

#include <algorithm>

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
        , render_backend_(create_render_backend())
        , render_device_(create_render_device(desc.device_select_fn))
        , render_pass_(create_render_pass())
        , command_allocator_(create_command_allocator())
        , shader_manager_(device())
        , render_size_(desc.render_size)
        , viewport_(Vector2_f{}, vector_cast<float>(render_size_), {0.f, 1.f})
        , scissor_({}, render_size_)
        , frames_([this] { return create_frame_data(); })
    {
    }

    Renderer::~Renderer()
    {
        device()->wait_idle();
    }

    QuadRenderer Renderer::create_quad_renderer()
    {
        return {device(), &shader_manager_, render_pass_};
    }

    void Renderer::begin()
    {
        auto& frame = frames_.get(current_frame_index_);

        // Wait for frame to finish
        device()->wait_for_fence(frame.fence);

        // Flush all destroyed resources every frames_in_flight frames
        if (current_frame_index_ == 0) {
            device()->destroy_flush();
        }

        // Reset command list
        frame.command_list->reset();

        // Begin command list
        frame.command_list->begin();

        // Begin render pass
        frame.command_list->begin_render_pass({
            .render_pass = render_pass_,
            .framebuffer = frame.render_target,
            .render_area = {
                .offset = {0, 0},
                .size = render_size_,
            },
            .clear_color = colors::magenta,
        });

        // Set the viewports
        frame.command_list->set_viewports(viewport_);

        // Set the scissors
        frame.command_list->set_scissors(scissor_);
    }

    void Renderer::end()
    {
        auto& frame = frames_.get(current_frame_index_);

        // Finish render pass
        frame.command_list->end_render_pass();

        // End command list recording
        frame.command_list->end();

        // Submit command buffer
        device()->submit(
            {
                .queue_type = CommandQueueType::Graphics,
                .command_lists = {{frame.command_list.get()}},
                .signal_semaphores = {{frame.render_semaphore}},
            },
            frame.fence);

        // Advance to next frame
        advance_frame();
    }

    void Renderer::render(QuadRenderer& quad_renderer)
    {
        quad_renderer.flush(current_context());
    }

    RenderWindow Renderer::create_render_window(Window& window, bool vsync)
    {
        return {device(), &window, vsync, command_allocator_.get()};
    }

    void Renderer::present(RenderWindow& render_window)
    {
        const auto& frame = frames_.get(previous_frame_index_);
        render_window.present({
            .frame_index = previous_frame_index_,
            .source_image = frame.render_image_view,
            .source_image_layout = ImageLayout::ShaderReadOnly,
            .wait_semaphores = {{frame.render_semaphore}},
        });
    }

    ImGuiContext Renderer::imgui_init(Window& window)
    {
        ORION_ASSERT(!ImGui_ImpOrion_Is_Initialized() && "ImGui is already initialized");

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        // Setup Platform/Renderer backends
        ImGui_ImplOrion_Init({
            .window = &window,
            .device = device(),
            .command_allocator = command_allocator_.get(),
            .render_pass = render_pass_,
            .shader_manager = &shader_manager_,
        });
        return {};
    }

    void Renderer::imgui_new_frame()
    {
        ORION_ASSERT(ImGui_ImpOrion_Is_Initialized() && "ImGui is not initialized. Did you call Renderer::imgui_init()");
        ImGui_ImplOrion_NewFrame(current_frame_index_);
        ImGui::NewFrame();
    }

    void Renderer::imgui_render()
    {
        ORION_ASSERT(ImGui_ImpOrion_Is_Initialized() && "ImGui is not initialized. Did you call Renderer::imgui_init()");
        ImGui::Render();
        ImGui_ImplOrion_RenderDrawData(ImGui::GetDrawData(), frames_.get(current_frame_index_).command_list.get());
    }

    spdlog::logger* Renderer::logger()
    {
        static const auto logger = create_logger("orion-renderer", ORION_RENDERER_LOG_LEVEL);
        return logger.get();
    }

    std::unique_ptr<RenderBackend> Renderer::create_render_backend() const
    {
        ORION_ASSERT(backend_module_.is_loaded());

        auto* fn_create_render_backend = backend_module_.load_symbol<CreateOrionRenderBackendFn>("create_orion_render_backend");
        if (fn_create_render_backend == nullptr) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to load function 'create_orion_render_backend' from module {}", backend_module_.filename());
            throw std::runtime_error("failed to load create_orion_render_backend()");
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Symbol create_orion_render_backend() loaded at {}", fmt::ptr(fn_create_render_backend));

        RenderBackend* render_backend = fn_create_render_backend();
        if (render_backend == nullptr) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to create render backend");
            throw std::runtime_error("failed to create render backend");
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Render backend created");

        return std::unique_ptr<RenderBackend>{render_backend};
    }

    std::unique_ptr<RenderDevice> Renderer::create_render_device(SelectPhysicalDeviceFn device_select_fn) const
    {
        ORION_ASSERT(render_backend_ != nullptr);

        const auto physical_devices = render_backend_->enumerate_physical_devices();
        if (physical_devices.empty()) {
            throw std::runtime_error("no physical devices found");
        }
        SPDLOG_LOGGER_INFO(logger(), "Found {} physical device(s):", physical_devices.size());
        for (const auto& device : physical_devices) {
            SPDLOG_LOGGER_INFO(logger(), "{}", device.name);
            SPDLOG_LOGGER_INFO(logger(), "-- Index: {}", device.index);
            SPDLOG_LOGGER_INFO(logger(), "-- Type: {}", device.type);
        }

        const auto physical_device_index = [&]() {
            if (device_select_fn != nullptr) {
                return device_select_fn(physical_devices);
            }
            return physical_devices[0].index;
        }();

        if (physical_device_index == invalid_physical_device_index) {
            throw std::runtime_error("no suitable physical device found");
        }

        SPDLOG_LOGGER_INFO(logger(), "Using physical device with index {}", physical_device_index);

        auto render_device = render_backend_->create_device(physical_device_index);
        SPDLOG_LOGGER_DEBUG(logger(), "Render device created");
        return render_device;
    }

    std::unique_ptr<CommandAllocator> Renderer::create_command_allocator() const
    {
        return device()->create_command_allocator({
            .queue_type = CommandQueueType::Graphics,
            .reset_command_buffer = true,
        });
    }

    RenderPassHandle Renderer::create_render_pass() const
    {
        return device()->create_render_pass({
            .color_attachments = {{
                AttachmentDesc{
                    .format = Format::B8G8R8A8_Srgb,
                    .load_op = AttachmentLoadOp::Clear,
                    .store_op = AttachmentStoreOp::Store,
                    .initial_layout = ImageLayout::Undefined,
                    .layout = ImageLayout::ColorAttachment,
                    .final_layout = ImageLayout::ShaderReadOnly,
                },
            }},
            .input_attachments = {},
            .bind_point = PipelineBindPoint::Graphics,
        });
    }

    Renderer::FrameData Renderer::create_frame_data() const
    {
        auto image = device()->create_image({
            .type = ImageType::Image2D,
            .format = Format::B8G8R8A8_Srgb,
            .size = vec3(render_size_, 1u),
            .tiling = ImageTiling::Optimal,
            .usage = ImageUsageFlags::ColorAttachment | ImageUsageFlags::SampledImage | ImageUsageFlags::TransferSrc,
            .host_visible = false,
        });
        auto image_view = device()->create_image_view({
            .image = image,
            .type = ImageViewType::View2D,
            .format = Format::B8G8R8A8_Srgb,
        });
        auto render_target = device()->create_framebuffer({
            .render_pass = render_pass_,
            .image_views = {&image_view, 1},
            .size = render_size_,
        });

        auto command_list = command_allocator_->create_command_list();
        auto present_command = command_allocator_->create_command_list();
        return {
            .render_image = image,
            .render_image_view = image_view,
            .render_target = render_target,
            .command_list = std::move(command_list),
            .fence = device()->create_fence({.start_finished = true}),
            .render_semaphore = device()->create_semaphore(),
        };
    }

    void Renderer::advance_frame()
    {
        previous_frame_index_ = current_frame_index_;
        current_frame_index_ = (current_frame_index_ + frame_index_t{1}) % frames_in_flight;
    }

    RenderContext Renderer::current_context() const
    {
        return {frames_.get(current_frame_index_).command_list.get(), current_frame_index_};
    }
} // namespace orion
