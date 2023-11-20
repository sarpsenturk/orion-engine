#include "orion-renderer/renderer.h"

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
        , shader_manager_(device())
        , render_size_(desc.render_size)
        , render_pass_(create_render_pass())
        , triangle_pipeline_layout_(create_triangle_pipeline_layout())
        , triangle_shader_effect_(shader_manager_.load_shader_effect("triangle"))
        , triangle_pipeline_(create_triangle_pipeline())
        , frames_(create_frame_data())
    {
    }

    void Renderer::begin()
    {
        auto& frame = current_frame();

        // Wait for frame to finish
        device()->wait_for_job(frame.render_job);

        // Reset command allocator
        frame.command_allocator->reset();

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
    }

    void Renderer::end()
    {
        auto& frame = current_frame();

        // Finish render pass
        frame.command_list->end_render_pass();

        // End command list recording
        frame.command_list->end();

        // Submit command buffer
        const auto command_lists = std::array{frame.command_list.get()};
        device()->submit({.queue_type = CommandQueueType::Graphics, .command_lists = command_lists, .job = frame.render_job});

        // Advance to next frame
        advance_frame();
    }

    void Renderer::draw(const Scene& scene)
    {
    }

    void Renderer::draw_test_triangle()
    {
        auto& frame = current_frame();

        // Bind the triangle pipeline
        frame.command_list->bind_pipeline({.pipeline = triangle_pipeline_, .bind_point = PipelineBindPoint::Graphics});

        // Set the viewports
        frame.command_list->set_viewports(Viewport{
            .position = Vector2_f{},
            .size = vector_cast<float>(render_size_),
            .depth = {0.f, 1.f},
        });

        // Set the scissors
        frame.command_list->set_scissors(Scissor{
            .offset = {},
            .size = render_size_,
        });

        // Make draw call
        frame.command_list->draw({.vertex_count = 3, .instance_count = 1, .first_vertex = 0, .first_instance = 0});
    }

    spdlog::logger* Renderer::logger()
    {
        static const auto logger = create_logger("orion-renderer", ORION_RENDERER_LOG_LEVEL);
        return logger.get();
    }

    void Renderer::advance_frame() noexcept
    {
        previous_frame_index_ = current_frame_index_;
        current_frame_index_ = (current_frame_index_ + 1) % frames_in_flight;
    }

    std::unique_ptr<RenderBackend> Renderer::create_render_backend() const
    {
        ORION_ASSERT(backend_module_.is_loaded());

        auto* fn_create_render_backend = backend_module_.load_symbol<pfnCreateRenderBackend>("create_render_backend");
        if (fn_create_render_backend == nullptr) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to load function 'create_render_backend' from module {}", backend_module_.filename());
            throw std::runtime_error("failed to load create_render_backend()");
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Symbol create_render_backend() loaded at {}", fmt::ptr(fn_create_render_backend));

        RenderBackend* render_backend = fn_create_render_backend();
        if (render_backend == nullptr) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to create render backend");
            throw std::runtime_error("failed to create render backend");
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Render backend created");

        return std::unique_ptr<RenderBackend>{render_backend};
    }

    std::unique_ptr<RenderDevice> Renderer::create_render_device(pfnSelectPhysicalDevice device_select_fn) const
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
        SPDLOG_LOGGER_INFO(logger(), "Using physical device with index {}", physical_device_index);

        auto render_device = render_backend_->create_device(physical_device_index);
        SPDLOG_LOGGER_DEBUG(logger(), "Render device created");
        return render_device;
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
                .final_layout = ImageLayout::General,
            },
        };
        return device()->create_render_pass({
            .color_attachments = color_attachments,
            .input_attachments = {},
            .bind_point = PipelineBindPoint::Graphics,
        });
    }

    PipelineLayoutHandle Renderer::create_triangle_pipeline_layout() const
    {
        return device()->create_pipeline_layout({.descriptors = {}, .push_constants = {}});
    }

    PipelineHandle Renderer::create_triangle_pipeline() const
    {
        const auto color_blend_attachments = std::array{
            BlendAttachmentDesc{.enable_blend = false},
        };
        return device()->create_graphics_pipeline({
            .shaders = triangle_shader_effect_.shader_stages(),
            .vertex_bindings = {},
            .pipeline_layout = triangle_pipeline_layout_,
            .color_blend = {.attachments = color_blend_attachments, .blend_constants = {}},
            .render_pass = render_pass_,
        });
    }

    Renderer::FrameDataArr Renderer::create_frame_data() const
    {
        FrameDataArr frame_data;
        auto generate_frame_data = [this]() -> FrameData {
            auto image = device()->create_image({
                .type = ImageType::Image2D,
                .format = Format::B8G8R8A8_Srgb,
                .size = vec3(render_size_, 1u),
                .tiling = ImageTiling::Optimal,
                .usage = ImageUsageFlags::ColorAttachment | ImageUsageFlags::InputAttachment,
            });
            auto image_view = device()->create_image_view({
                .image = image,
                .type = ImageViewType::View2D,
                .format = Format::B8G8R8A8_Srgb,
            });
            const auto color_attachments = std::array{
                AttachmentDesc{
                    .format = Format::B8G8R8A8_Srgb,
                    .load_op = AttachmentLoadOp::Clear,
                    .store_op = AttachmentStoreOp::Store,
                    .initial_layout = ImageLayout::Undefined,
                    .layout = ImageLayout::ColorAttachment,
                    .final_layout = ImageLayout::ShaderReadOnly,
                },
            };
            auto render_target = device()->create_framebuffer({
                .render_pass = render_pass_,
                .image_views = {&image_view, 1},
                .size = render_size_,
            });
            auto command_allocator = device()->create_command_allocator(CommandQueueType::Graphics);
            auto command_list = command_allocator->create_command_list();
            return {
                .render_image = image,
                .render_image_view = image_view,
                .render_target = render_target,
                .command_allocator = std::move(command_allocator),
                .command_list = std::move(command_list),
                .render_job = device()->create_job({.start_finished = true}),
            };
        };
        for (int i = 0; i < frames_in_flight; ++i) {
            frame_data.push_back(generate_frame_data());
        }
        return frame_data;
    }
} // namespace orion
