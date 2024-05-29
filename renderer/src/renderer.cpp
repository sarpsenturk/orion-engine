#include "orion-renderer/renderer.h"

#include "orion-renderer/colors.h"

#include "orion-renderapi/config.h"

#include "orion-core/window.h"

#include "orion-utils/type.h"

#include <algorithm>
#include <array>

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
    } // namespace

    std::uint16_t RenderCommand::material() const noexcept
    {
        return static_cast<std::uint16_t>(key >> 48);
    }

    std::uint16_t RenderCommand::mesh() const noexcept
    {
        return static_cast<std::uint16_t>((key >> 32) & 0xFFFF);
    }

    Renderer::Renderer(const RendererDesc& desc)
        : render_backend_module_(load_backend_module(desc.backend))
        , render_backend_(create_render_backend(render_backend_module_))
        , render_device_(create_render_device(render_backend_.get()))
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
        , render_context_(render_device_.get(), {desc.render_size, frame_descriptor_layout_, present_descriptor_layout_, present_sampler_})
        , mesh_builder_(&render_context_)
        , material_builder_(&render_context_, material_descriptor_layout_)
    {
        SPDLOG_LOGGER_INFO(logger(), "Renderer initialized");
        SPDLOG_LOGGER_INFO(logger(), "Render Backend Info:");
        SPDLOG_LOGGER_INFO(logger(), "- backend module: {}", render_backend_module_.filename());
        SPDLOG_LOGGER_TRACE(logger(), "- render backend: {}", fmt::ptr(render_backend_));
        SPDLOG_LOGGER_TRACE(logger(), "- render device: {}", fmt::ptr(render_device_));
    }

    void Renderer::draw(const RenderObj& obj)
    {
        const auto material = add_material(obj.material);
        const auto mesh = add_mesh(obj.mesh);
        add_transform(obj.transform);
        const auto key = (std::uint64_t{material} << 48ull) | (std::uint64_t{mesh} << 32ull);
        commands_.push_back({key});
    }

    void Renderer::render(const Matrix4_f& view_projection)
    {
        auto& frame = render_context_.current_frame();

        render_device_->wait_for_fence(frame.present_fence);

        if (render_context_.current_frame_index() == 0) {
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

        bind_view(render_command, view_projection);

        std::sort(commands_.begin(), commands_.end());

        std::uint16_t material = UINT16_MAX;
        std::uint16_t mesh = UINT16_MAX;
        for (std::size_t object = 0; auto command : commands_) {
            if (const auto index = command.material(); index != material) {
                material = index;
                bind_material(render_command, material);
            }

            if (const auto index = command.mesh(); index != mesh) {
                mesh = index;
                bind_mesh(render_command, mesh);
            }

            bind_transform(render_command, object++);

            render_command->draw_indexed({
                .index_count = meshes_[mesh]->index_count(),
                .instance_count = 1,
                .first_index = 0,
                .vertex_offset = 0,
                .first_instance = 0,
            });
        }
        clear_scene();

        render_command->end_render_pass();
        render_command->end();

        const auto render_semaphore = frame.render_semaphore;
        const auto submit = SubmitDesc{
            .queue_type = CommandQueueType::Graphics,
            .command_lists = {&render_command, 1},
            .signal_semaphores = {&render_semaphore, 1},
        };
        render_device_->submit(submit, frame.render_fence);

        render_context_.advance_frame();
    }

    void Renderer::present_to(const RenderTarget& render_target)
    {
        auto& frame = render_context_.previous_frame();

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
        const auto present_semaphore = render_context_.previous_frame().present_semaphore;
        render_window.present({&present_semaphore, 1});
    }

    RenderWindow Renderer::create_render_window(const Window& window)
    {
        return ::orion::create_render_window(render_device_.get(), &window);
    }

    std::uint16_t Renderer::add_material(const Material* material)
    {
        if (const auto iter = std::ranges::find(materials_, material); iter != materials_.end()) {
            return static_cast<std::uint16_t>(std::distance(materials_.begin(), iter));
        }
        if (materials_.size() >= UINT16_MAX) {
            throw std::runtime_error{"can't have a scene with more than UINT16_MAX materials"};
        }
        const auto id = static_cast<std::uint16_t>(materials_.size());
        materials_.push_back(material);
        return id;
    }

    std::uint16_t Renderer::add_mesh(const Mesh* mesh)
    {
        if (const auto iter = std::ranges::find(meshes_, mesh); iter != meshes_.end()) {
            return static_cast<std::uint16_t>(std::distance(meshes_.begin(), iter));
        }
        if (materials_.size() >= UINT16_MAX) {
            throw std::runtime_error{"can't have a scene with more than UINT16_MAX meshes"};
        }
        const auto id = static_cast<std::uint16_t>(meshes_.size());
        meshes_.push_back(mesh);
        return id;
    }

    std::size_t Renderer::add_transform(const Matrix4_f* transform)
    {
        const auto id = transforms_.size();
        transforms_.push_back(transform);
        return id;
    }

    void Renderer::clear_scene()
    {
        commands_.clear();
        materials_.clear();
        meshes_.clear();
        transforms_.clear();
    }

    void Renderer::bind_view(CommandList* command_list, const Matrix4_f& view_projection)
    {
        const auto cbuffer = render_context_.frame_cbuffer();
        void* dst = render_device_->map(cbuffer);
        std::memcpy(dst, &view_projection, sizeof(view_projection));
        render_device_->unmap(cbuffer);
        command_list->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = pipeline_layout_,
            .index = descriptor_index_frame,
            .descriptor = render_context_.frame_descriptor(),
        });
    }

    void Renderer::bind_material(CommandList* command_list, std::uint16_t index)
    {
        const auto* material = materials_[index];
        const auto* effect = material->effect();
        command_list->bind_pipeline({.pipeline = effect->pipeline(), .bind_point = PipelineBindPoint::Graphics});
        command_list->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = pipeline_layout_,
            .index = descriptor_index_material,
            .descriptor = material->descriptor(),
        });
    }

    void Renderer::bind_mesh(CommandList* command_list, std::uint16_t index)
    {
        const auto* mesh = meshes_[index];
        command_list->bind_vertex_buffer({.vertex_buffer = mesh->vertex_buffer(), .offset = 0});
        command_list->bind_index_buffer({.index_buffer = mesh->index_buffer(), .offset = 0, .index_type = vertex_index_type});
    }

    void Renderer::bind_transform(CommandList* command_list, std::size_t object)
    {
    }
} // namespace orion
