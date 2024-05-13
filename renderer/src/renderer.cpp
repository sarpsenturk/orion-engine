#include "orion-renderer/renderer.h"

#include "orion-renderer/colors.h"

#include "orion-renderapi/config.h"

#include <algorithm>

#ifndef ORION_RENDERER_LOG_LEVEL
    #define ORION_RENDERER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    namespace
    {
        auto* logger()
        {
            static const auto logger = create_logger("orion-renderer", ORION_RENDERER_LOG_LEVEL);
            return logger.get();
        }

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

        Module load_backend_module(const char* name)
        {
            if (name == nullptr) {
                return Module{default_backend_module(current_platform)};
            }
            return Module{name};
        }

        std::unique_ptr<RenderBackend> create_render_backend(const Module& backend_module)
        {
            auto* create_orion_render_backend_fn = backend_module.load_symbol<CreateOrionRenderBackendFn>("create_orion_render_backend");
            if (create_orion_render_backend_fn == nullptr) {
                SPDLOG_LOGGER_ERROR(logger(), "Failed to load function pointer 'create_orion_render_backend");
                throw std::runtime_error{"renderer initialization failed"};
            }
            return std::unique_ptr<RenderBackend>{create_orion_render_backend_fn()};
        }

        std::unique_ptr<RenderDevice> create_render_device(RenderBackend* render_backend)
        {
            const auto physical_devices = render_backend->enumerate_physical_devices();
            if (auto iter = std::ranges::find_if(physical_devices, is_discrete_gpu); iter != physical_devices.end()) {
                return render_backend->create_device(iter->index);
            }
            throw std::runtime_error{"no supported GPU found"};
        }

        FrameInFlight create_frame(RenderDevice* device, const Vector2_u& render_size)
        {
            auto command_allocator = device->create_command_allocator({.queue_type = CommandQueueType::Graphics, .reset_command_buffer = false});
            auto command_list = command_allocator->create_command_list();
            return FrameInFlight{
                .command_allocator = std::move(command_allocator),
                .command_list = std::move(command_list),
                .render_fence = device->create_fence({.start_finished = true}),
                .render_semaphore = device->create_semaphore(),
                .render_target = create_render_target(device, {.size = render_size}),
            };
        }

        static_vector<FrameInFlight, frames_in_flight> create_frames_in_flight(RenderDevice* device, const Vector2_u& render_size)
        {
            static_vector<FrameInFlight, frames_in_flight> frames;
            for (int i = 0; i < frames_in_flight; ++i) {
                frames.push_back(create_frame(device, render_size));
            }
            return frames;
        }
    } // namespace

    Renderer::Renderer(const RendererDesc& desc)
        : render_backend_module_(load_backend_module(desc.backend))
        , render_backend_(create_render_backend(render_backend_module_))
        , render_device_(create_render_device(render_backend_.get()))
        , command_allocator_(render_device_->create_command_allocator({.queue_type = CommandQueueType::Graphics, .reset_command_buffer = false}))
        , mesh_builder_(render_device_.get(), command_allocator_.get())
        , shader_reflector_(render_backend_->create_shader_reflector())
        , effect_compiler_(render_device_.get(), shader_reflector_.get())
        , render_size_(desc.render_size)
        , frames_in_flight_(create_frames_in_flight(render_device_.get(), desc.render_size))
    {
        SPDLOG_LOGGER_INFO(logger(), "Renderer initialized");
        SPDLOG_LOGGER_INFO(logger(), "Render Backend Info:");
        SPDLOG_LOGGER_INFO(logger(), "- backend module: {}", render_backend_module_.filename());
        SPDLOG_LOGGER_TRACE(logger(), "- render backend: {}", fmt::ptr(render_backend_));
        SPDLOG_LOGGER_TRACE(logger(), "- render device: {}", fmt::ptr(render_device_));
    }

    void Renderer::draw(RenderObj obj)
    {
        objects_.push_back(obj);
    }

    void Renderer::render()
    {
        auto& frame = current_frame();

        const auto render_fence = frame.render_fence;
        render_device_->wait_for_fence(render_fence);

        if (current_frame_index_ == 0) {
            render_device_->destroy_flush();
        }

        auto* command_allocator = frame.command_allocator.get();
        command_allocator->reset();

        auto* command_list = frame.command_list.get();

        const auto& render_target = frame.render_target;
        command_list->begin();
        command_list->begin_render_pass({
            .render_pass = render_target.render_pass(),
            .framebuffer = render_target.framebuffer(),
            .render_area = {
                .offset = {0, 0},
                .size = render_size_,
            },
            .clear_color = colors::magenta,
        });

        command_list->set_viewports(Viewport{
            .position = {0.f, 0.f},
            .size = vector_cast<float>(render_size_),
            .depth = {0.f, 1.f},
        });

        command_list->set_scissors(Scissor{
            .offset = {0, 0},
            .size = render_size_,
        });

        // TODO: Sort objects based on materials
        for (const auto& obj : objects_) {
            const auto* material = obj.material;
            const auto* effect = material->effect();

            command_list->bind_pipeline({.pipeline = effect->pipeline(), .bind_point = PipelineBindPoint::Graphics});

            const auto* mesh = obj.mesh;
            command_list->bind_vertex_buffer({.vertex_buffer = mesh->vertex_buffer(), .offset = 0});
            command_list->bind_index_buffer({.index_buffer = mesh->index_buffer(), .offset = 0, .index_type = vertex_index_type});

            command_list->draw_indexed({
                .index_count = mesh->index_count(),
                .instance_count = 1,
                .first_index = 0,
                .vertex_offset = 0,
                .first_instance = 0,
            });
        }

        command_list->end_render_pass();
        command_list->end();

        const auto render_semaphore = frame.render_semaphore;
        const auto submit = SubmitDesc{
            .queue_type = CommandQueueType::Graphics,
            .command_lists = {&command_list, 1},
        };
        render_device_->submit(submit, render_fence);

        advance_frame();
    }

    void Renderer::advance_frame()
    {
        previous_frame_index_ = current_frame_index_;
        current_frame_index_ = (current_frame_index_ + 1) % frames_in_flight;
    }
} // namespace orion
