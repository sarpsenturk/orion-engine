#include "orion-renderer/renderer.h"

#include "orion-renderer/colors.h"

#include "orion-renderapi/config.h"

#include "orion-core/window.h"

#include <algorithm>
#include <array>
#include <span>

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

        DescriptorPoolHandle create_descriptor_pool(RenderDevice* device)
        {
            return device->create_descriptor_pool({
                .max_descriptors = 16,
                .flags = DescriptorPoolFlags::FreeDescriptors,
                .sizes = {{
                    DescriptorPoolSize{
                        .type = DescriptorType::Sampler,
                        .count = frames_in_flight,
                    },
                    DescriptorPoolSize{
                        .type = DescriptorType::SampledImage,
                        .count = frames_in_flight,
                    },
                }},
            });
        }

        DescriptorLayoutHandle create_frame_descriptor_layout(RenderDevice* device)
        {
            const auto bindings = std::array{
                DescriptorBindingDesc{
                    .type = DescriptorType::ConstantBuffer,
                    .shader_stages = ShaderStageFlags::All,
                    .count = 1,
                },
            };
            return device->create_descriptor_layout({bindings});
        }

        DescriptorLayoutHandle create_material_descriptor_layout(RenderDevice* device)
        {
            const auto bindings = std::array{
                DescriptorBindingDesc{
                    .type = DescriptorType::ConstantBuffer,
                    .shader_stages = ShaderStageFlags::All,
                    .count = 1,
                },
            };
            return device->create_descriptor_layout({bindings});
        }

        DescriptorLayoutHandle create_object_descriptor_layout(RenderDevice* device)
        {
            const auto bindings = std::array{
                DescriptorBindingDesc{
                    .type = DescriptorType::ConstantBuffer,
                    .shader_stages = ShaderStageFlags::All,
                    .count = 1,
                },
            };
            return device->create_descriptor_layout({bindings});
        }

        DescriptorLayoutHandle create_present_descriptor_layout(RenderDevice* device)
        {
            const auto bindings = std::array{
                DescriptorBindingDesc{
                    .type = DescriptorType::SampledImage,
                    .shader_stages = ShaderStageFlags::Pixel,
                    .count = 1,
                },
                DescriptorBindingDesc{
                    .type = DescriptorType::Sampler,
                    .shader_stages = ShaderStageFlags::Pixel,
                    .count = 1,
                },
            };
            return device->create_descriptor_layout({bindings});
        }

        PipelineLayoutHandle create_present_pipeline_layout(RenderDevice* device, DescriptorLayoutHandle descriptor_layout)
        {
            return device->create_pipeline_layout({.descriptors = {&descriptor_layout, 1}});
        }

        SamplerHandle create_present_sampler(RenderDevice* device)
        {
            return device->create_sampler({
                .filter = Filter::Nearest,
                .address_mode_u = AddressMode::Border,
                .address_mode_v = AddressMode::Border,
                .address_mode_w = AddressMode::Border,
                .mip_load_bias = 0.f,
                .max_anisotropy = 1.f,
                .compare_func = CompareFunc::Always,
                .min_lod = 0.f,
                .max_lod = 0.f,
            });
        }

        void init_present_descriptor(RenderDevice* device, DescriptorHandle present_descriptor, ImageViewHandle render_image, SamplerHandle sampler)
        {
            const auto render_output_image = ImageDescriptorDesc{
                .image_view_handle = render_image,
                .image_layout = ImageLayout::ShaderReadOnly,
            };
            const auto render_output_sampler = ImageDescriptorDesc{
                .sampler_handle = sampler,
            };
            const auto descriptor_writes = std::array{
                DescriptorWrite{
                    .binding = 0,
                    .descriptor_type = DescriptorType::SampledImage,
                    .images = {&render_output_image, 1},
                },
                DescriptorWrite{
                    .binding = 1,
                    .descriptor_type = DescriptorType::Sampler,
                    .images = {&render_output_sampler, 1},
                },
            };
            device->write_descriptor(present_descriptor, descriptor_writes);
        }

        struct FrameInFlightDesc {
            Vector2_u render_size;
            DescriptorPoolHandle descriptor_pool;
            DescriptorLayoutHandle present_descriptor_layout;
            SamplerHandle present_sampler;
        };

        FrameInFlight create_frame(RenderDevice* device, const FrameInFlightDesc& desc)
        {
            const auto render_target_desc = RenderTargetDesc{
                .size = desc.render_size,
                .image_usage = ImageUsageFlags::ColorAttachment | ImageUsageFlags::SampledImage,
                .initial_layout = ImageLayout::Undefined,
                .final_layout = ImageLayout::ShaderReadOnly,
            };
            auto render_target = create_render_target(device, render_target_desc);

            auto render_output_descriptor = device->create_descriptor(desc.present_descriptor_layout, desc.descriptor_pool);
            init_present_descriptor(device, render_output_descriptor, render_target.image_view(), desc.present_sampler);

            auto command_allocator = device->create_command_allocator({.queue_type = CommandQueueType::Graphics, .reset_command_buffer = false});
            auto render_command = command_allocator->create_command_list();
            auto present_command = command_allocator->create_command_list();
            return FrameInFlight{
                .command_allocator = std::move(command_allocator),
                .render_command = std::move(render_command),
                .render_fence = device->create_fence({.start_finished = false}),
                .render_semaphore = device->create_semaphore(),
                .render_target = std::move(render_target),
                .render_output_descriptor = render_output_descriptor,
                .present_command = std::move(present_command),
                .present_fence = device->create_fence({.start_finished = true}),
                .present_semaphore = device->create_semaphore(),
            };
        }

        static_vector<FrameInFlight, frames_in_flight> create_frames_in_flight(RenderDevice* device, const FrameInFlightDesc& desc)
        {
            static_vector<FrameInFlight, frames_in_flight> frames;
            for (int i = 0; i < frames_in_flight; ++i) {
                frames.push_back(create_frame(device, desc));
            }
            return frames;
        }
    } // namespace

    Renderer::Renderer(const RendererDesc& desc)
        : render_backend_module_(load_backend_module(desc.backend))
        , render_backend_(create_render_backend(render_backend_module_))
        , render_device_(create_render_device(render_backend_.get()))
        , command_allocator_(render_device_->create_command_allocator({.queue_type = CommandQueueType::Graphics, .reset_command_buffer = false}))
        , descriptor_pool_(create_descriptor_pool(render_device_.get()))
        , mesh_builder_(render_device_.get(), command_allocator_.get())
        , shader_reflector_(render_backend_->create_shader_reflector())
        , frame_descriptor_layout_(create_frame_descriptor_layout(render_device_.get()))
        , material_descriptor_layout_(create_material_descriptor_layout(render_device_.get()))
        , object_descriptor_layout_(create_object_descriptor_layout(render_device_.get()))
        , pipeline_layout_(render_device_->create_pipeline_layout({.descriptors = {{frame_descriptor_layout_, material_descriptor_layout_, object_descriptor_layout_}}}))
        , effect_compiler_(render_device_.get(), shader_reflector_.get(), pipeline_layout_)
        , render_size_(desc.render_size)
        , present_descriptor_layout_(create_present_descriptor_layout(render_device_.get()))
        , present_pipeline_layout_(create_present_pipeline_layout(render_device_.get(), present_descriptor_layout_))
        , present_effect_(EffectCompiler{render_device_.get(), shader_reflector_.get(), present_pipeline_layout_}.compile_file(input_file("assets/effects/present.ofx"), {.shader_base_path = render_backend_->shader_base_path()}))
        , present_sampler_(create_present_sampler(render_device_.get()))
        , frames_in_flight_(create_frames_in_flight(render_device_.get(), {desc.render_size, descriptor_pool_, present_descriptor_layout_, present_sampler_}))
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

        render_device_->wait_for_fence(frame.present_fence);

        if (current_frame_index_ == 0) {
            render_device_->destroy_flush();
        }

        auto* command_allocator = frame.command_allocator.get();
        command_allocator->reset();

        auto* render_command = frame.render_command.get();

        const auto& render_target = frame.render_target;
        render_command->begin();
        render_command->begin_render_pass({
            .render_pass = render_target.render_pass(),
            .framebuffer = render_target.framebuffer(),
            .render_area = {
                .offset = {0, 0},
                .size = render_size_,
            },
            .clear_color = colors::magenta,
        });

        render_command->set_viewports(Viewport{
            .position = {0.f, 0.f},
            .size = vector_cast<float>(render_size_),
            .depth = {0.f, 1.f},
        });

        render_command->set_scissors(Scissor{
            .offset = {0, 0},
            .size = render_size_,
        });

        // TODO: Sort objects based on materials
        for (const auto& obj : objects_) {
            const auto* material = obj.material;
            const auto* effect = material->effect();

            render_command->bind_pipeline({.pipeline = effect->pipeline(), .bind_point = PipelineBindPoint::Graphics});

            const auto* mesh = obj.mesh;
            render_command->bind_vertex_buffer({.vertex_buffer = mesh->vertex_buffer(), .offset = 0});
            render_command->bind_index_buffer({.index_buffer = mesh->index_buffer(), .offset = 0, .index_type = vertex_index_type});

            render_command->draw_indexed({
                .index_count = mesh->index_count(),
                .instance_count = 1,
                .first_index = 0,
                .vertex_offset = 0,
                .first_instance = 0,
            });
        }
        objects_.clear();

        render_command->end_render_pass();
        render_command->end();

        const auto render_semaphore = frame.render_semaphore;
        const auto submit = SubmitDesc{
            .queue_type = CommandQueueType::Graphics,
            .command_lists = {&render_command, 1},
            .signal_semaphores = {&render_semaphore, 1},
        };
        render_device_->submit(submit, frame.render_fence);

        advance_frame();
    }

    void Renderer::present_to(const RenderTarget& render_target)
    {
        auto& frame = previous_frame();

        // Wait for renderer to finish rendering
        render_device_->wait_for_fence(frame.render_fence);

        auto* present_command = frame.present_command.get();

        present_command->begin();
        present_command->begin_render_pass({
            .render_pass = render_target.render_pass(),
            .framebuffer = render_target.framebuffer(),
            .render_area = {
                .offset = {0, 0},
                .size = render_size_,
            },
        });
        present_command->bind_pipeline({.pipeline = present_effect_.pipeline(), .bind_point = PipelineBindPoint::Graphics});
        present_command->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = present_pipeline_layout_,
            .index = 0,
            .descriptor = frame.render_output_descriptor,
        });
        present_command->set_viewports(Viewport{
            .position = {0.f, 0.f},
            .size = vector_cast<float>(render_size_),
            .depth = {0.f, 1.f},
        });

        present_command->set_scissors(Scissor{
            .offset = {0, 0},
            .size = render_size_,
        });
        present_command->draw({.vertex_count = 3, .instance_count = 1, .first_vertex = 0, .first_instance = 0});
        present_command->end_render_pass();
        present_command->end();

        const auto render_semaphore = frame.render_semaphore;
        const auto present_semaphore = frame.present_semaphore;
        const auto submit = SubmitDesc{
            .queue_type = CommandQueueType::Graphics,
            .wait_semaphores = {&render_semaphore, 1},
            .command_lists = {&present_command, 1},
            .signal_semaphores = {&present_semaphore, 1},
        };
        render_device_->submit(submit, frame.present_fence);
    }

    void Renderer::present(RenderWindow& render_window)
    {
        present_to(render_window.acquire_render_target());
        const auto present_semaphore = previous_frame().present_semaphore;
        render_window.present({&present_semaphore, 1});
    }

    RenderWindow Renderer::create_render_window(const Window& window)
    {
        return ::orion::create_render_window(render_device_.get(), &window);
    }

    void Renderer::advance_frame()
    {
        previous_frame_index_ = current_frame_index_;
        current_frame_index_ = (current_frame_index_ + 1) % frames_in_flight;
    }
} // namespace orion
