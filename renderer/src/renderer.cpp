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
        , render_size_(desc.render_size)
        , render_pass_(create_render_pass())
        , shader_manager_(device())
        , frames_(create_frame_data())
    {
    }

    void Renderer::begin()
    {
        auto& frame = current_frame();

        frame.sprite_renderer.reset();
    }

    void Renderer::end()
    {
    }

    void Renderer::draw(const Scene& scene)
    {
        auto& frame = current_frame();
        // Render sprites
        {
            auto sprites = scene.view<TransformComponent, SpriteComponent>();
            sprites.each([&](auto entity, const auto& transform, const auto& sprite) {
                (void)entity;
                frame.sprite_renderer.draw({.sprite = &sprite, .position = transform.position()});
            });
        }
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
            return {
                .render_image = image,
                .render_image_view = image_view,
                .render_target = render_target,
                .command_allocator = device()->create_command_allocator(CommandQueueType::Graphics),
                .sprite_renderer = SpriteRenderer{{.device = device()}},
            };
        };
        for (int i = 0; i < frames_in_flight; ++i) {
            frame_data.push_back(generate_frame_data());
        }
        return frame_data;
    }
} // namespace orion
