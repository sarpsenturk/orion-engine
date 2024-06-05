#include "orion-renderer/renderer.h"

#include "orion-renderer/colors.h"
#include "orion-renderer/types.h"

#include "orion-renderapi/config.h"

#include "orion-core/window.h"

#include "orion-utils/assertion.h"
#include "orion-utils/type.h"

#include "imgui_impl_orion.h"

#include <algorithm>
#include <array>
#include <bit>
#include <limits>

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

        DescriptorLayoutHandle create_object_descriptor_layout(RenderDevice* device)
        {
            const auto bindings = std::array{
                DescriptorBindingDesc{
                    .type = DescriptorType::StorageBuffer,
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
                .mip_lod_bias = 0.f,
                .max_anisotropy = 1.f,
                .compare_func = CompareFunc::Always,
                .min_lod = 0.f,
                .max_lod = 0.f,
            });
        }

        DescriptorPoolHandle create_material_descriptor_pool(RenderDevice* device)
        {
            // TODO: For more than 1 material we need a dynamic way to handle this.
            //  We can't allocate a pool for UINT16_MAX descriptors with UINT16_MAX sizes each (I assume!??)
            //  Probably a free list approach where we create descriptor pools with N max_descriptors & N count for each DescriptorPoolSize
            //  and add them to a free list as materials are created would work.
            const auto max_materials = 2;
            return device->create_descriptor_pool({
                .max_descriptors = max_materials,
                .flags = {},
                .sizes = {{
                    DescriptorPoolSize{
                        .type = DescriptorType::ConstantBuffer,
                        .count = max_materials,
                    },
                    DescriptorPoolSize{
                        .type = DescriptorType::SampledImage,
                        .count = max_materials,
                    },
                    DescriptorPoolSize{
                        .type = DescriptorType::Sampler,
                        .count = max_materials,
                    },
                }},
            });
        }

        auto find_by_id(auto id)
        {
            return [id](const auto& pair) { return pair.first == id; };
        }
    } // namespace

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
        , present_effect_(EffectCompiler{render_device_.get(), shader_reflector_.get(), present_pipeline_layout_}.compile_file(input_file("assets/effects/present.ofx")))
        , present_sampler_(create_present_sampler(render_device_.get()))
        , render_context_(render_device_.get(), {desc.render_size, frame_descriptor_layout_, present_descriptor_layout_, object_descriptor_layout_, present_sampler_})
        , material_descriptor_pool_(create_material_descriptor_pool(render_device_.get()))
    {
        create_default_textures();

        SPDLOG_LOGGER_INFO(logger(), "Renderer initialized");
        SPDLOG_LOGGER_INFO(logger(), "Render Backend Info:");
        SPDLOG_LOGGER_INFO(logger(), "- backend module: {}", render_backend_module_.filename());
        SPDLOG_LOGGER_TRACE(logger(), "- render backend: {}", fmt::ptr(render_backend_));
        SPDLOG_LOGGER_TRACE(logger(), "- render device: {}", fmt::ptr(render_device_));
    }

    void Renderer::draw(const RenderObj& obj)
    {
        const auto object_index = objects_.size();
        objects_.push_back(obj);
        void* dst = render_device_->map(render_context_.object_buffer());
        const auto object_offset = object_index * sizeof(RenderObjBuffer);
        dst = static_cast<char*>(dst) + object_offset;
        // TODO: This is dangerous. If we change RenderObjBuffer but forget to memcpy the required bytes here
        //  code will break.
        std::memcpy(dst, obj.transform, sizeof(RenderObjBuffer));
        render_device_->unmap(render_context_.object_buffer());
    }

    void Renderer::render(const Camera& camera)
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

        const auto cbuffer = render_context_.frame_cbuffer();
        void* dst = render_device_->map(cbuffer);
        std::memcpy(dst, &camera.view_projection(), sizeof(Matrix4_f));
        render_device_->unmap(cbuffer);
        render_command->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = pipeline_layout_,
            .index = descriptor_index_frame,
            .descriptor = render_context_.frame_descriptor(),
        });

        render_command->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = pipeline_layout_,
            .index = descriptor_index_object,
            .descriptor = render_context_.object_descriptor(),
        });

        for (std::size_t index = 0; const auto& obj : objects_) {
            const auto* material = find_material(obj.material);
            const auto* effect = material->effect();
            render_command->bind_pipeline({.pipeline = effect->pipeline(), .bind_point = PipelineBindPoint::Graphics});
            render_command->bind_descriptor({
                .bind_point = PipelineBindPoint::Graphics,
                .pipeline_layout = pipeline_layout_,
                .index = descriptor_index_material,
                .descriptor = material->descriptor(),
            });

            const auto* mesh = find_mesh(obj.mesh);
            render_command->bind_vertex_buffer({.vertex_buffer = mesh->vertex_buffer(), .offset = 0});
            render_command->bind_index_buffer({.index_buffer = mesh->index_buffer(), .offset = 0, .index_type = vertex_index_type});

            render_command->draw_indexed({
                .index_count = mesh->index_count(),
                .instance_count = 1,
                .first_index = 0,
                .vertex_offset = 0,
                .first_instance = static_cast<std::uint32_t>(index++),
            });
        }
        objects_.clear();

        if (imgui_) {
            ImGui_ImplOrion_NewFrame();
            ImGui::NewFrame();
            imgui_->on_draw();
            ImGui::Render();
            ImGui_ImplOrion_RenderDrawData(ImGui::GetDrawData());
        }

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

    void Renderer::imgui_init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;
        ImGui_ImplOrion_Init({render_device_.get(), &render_context_, render_size_});
    }

    std::pair<mesh_id_t, Mesh*> Renderer::create_mesh(std::span<const Vertex> vertices, std::span<const vertex_index_t> indices)
    {
        SPDLOG_LOGGER_TRACE(logger(), "Creating mesh...");
        ORION_ASSERT(meshes_.size() < std::numeric_limits<mesh_id_t>::max());

        // Create vertex buffer
        auto vertex_buffer = render_device_->create_buffer({
            .size = vertices.size_bytes(),
            .usage = GPUBufferUsageFlags::VertexBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = false,
        });
        SPDLOG_LOGGER_TRACE(logger(), "Created vertex buffer with size: {} bytes", vertices.size_bytes());

        // Create index buffer
        auto index_buffer = render_device_->create_buffer({
            .size = indices.size_bytes(),
            .usage = GPUBufferUsageFlags::IndexBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = false,
        });
        SPDLOG_LOGGER_TRACE(logger(), "Created index buffer with size: {} bytes", indices.size_bytes());

        render_context_.copy_buffer_staging({{
            CopyBufferStaging{
                .bytes = std::as_bytes(vertices),
                .dst = vertex_buffer,
            },
            CopyBufferStaging{
                .bytes = std::as_bytes(indices),
                .dst = index_buffer,
            },
        }});
        SPDLOG_LOGGER_TRACE(logger(), "Data copied to GPU.");
        SPDLOG_LOGGER_DEBUG(logger(), "Mesh created.");

        const auto id = static_cast<mesh_id_t>(meshes_.size());
        auto& mesh = meshes_.emplace_back(std::piecewise_construct,
                                          std::forward_as_tuple(id),
                                          std::forward_as_tuple(render_device_->to_unique(vertex_buffer),
                                                                render_device_->to_unique(index_buffer),
                                                                static_cast<std::uint32_t>(indices.size())));
        return std::make_pair(id, &mesh.second);
    }

    std::pair<material_id_t, Material*> Renderer::create_material(const Effect* effect, const MaterialData& data)
    {
        ORION_ASSERT(materials_.size() < std::numeric_limits<material_id_t>::max());

        // Create constant buffer
        const auto buffer_size = sizeof(MaterialData);
        auto constant_buffer = render_device_->create_buffer({
            .size = buffer_size,
            .usage = GPUBufferUsageFlags::ConstantBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = true,
        });

        // Upload material data to GPU
        {
            const auto bytes = std::bit_cast<std::array<std::byte, sizeof(MaterialData)>>(data);
            render_context_.copy_buffer_staging({.bytes = bytes, .dst = constant_buffer});
        }

        auto descriptor = render_device_->create_descriptor(material_descriptor_layout_, material_descriptor_pool_);
        // Write buffer, texture & sampler to descriptor
        {
            const auto buffer_write = BufferDescriptorDesc{.buffer_handle = constant_buffer, .region = {.size = buffer_size, .offset = 0}};
            const auto texture_write = ImageDescriptorDesc{.image_view_handle = textures_[0].second.image_view(), .image_layout = ImageLayout::ShaderReadOnly};
            const auto sampler_write = ImageDescriptorDesc{.sampler_handle = textures_[0].second.sampler()};
            const auto writes = std::array{
                DescriptorWrite{
                    .binding = 0,
                    .descriptor_type = DescriptorType::ConstantBuffer,
                    .array_start = 0,
                    .buffers = {&buffer_write, 1},
                },
                DescriptorWrite{
                    .binding = 1,
                    .descriptor_type = DescriptorType::SampledImage,
                    .array_start = 0,
                    .images = {&texture_write, 1},
                },
                DescriptorWrite{
                    .binding = 2,
                    .descriptor_type = DescriptorType::Sampler,
                    .array_start = 0,
                    .images = {&sampler_write, 1},
                },
            };
            render_device_->write_descriptor(descriptor, writes);
        }

        const auto id = static_cast<material_id_t>(materials_.size());
        auto& material = materials_.emplace_back(std::piecewise_construct,
                                                 std::forward_as_tuple(id),
                                                 std::forward_as_tuple(effect,
                                                                       render_device_->to_unique(constant_buffer),
                                                                       render_device_->to_unique(descriptor),
                                                                       data));
        return std::make_pair(id, &material.second);
    }

    std::pair<texture_id_t, Texture*> Renderer::create_texture(TextureInfo info, std::span<const std::byte> bytes)
    {
        ORION_ASSERT(textures_.size() < std::numeric_limits<texture_id_t>::max());

        // Create image
        auto image = render_device_->create_image({
            .type = to_image_type(info.type),
            .format = info.format,
            .size = {info.width, info.height, 1},
            .tiling = ImageTiling::Optimal,
            .usage = ImageUsageFlags::SampledImage | (!info.host_visible ? ImageUsageFlags::TransferDst : ImageUsageFlags{}),
            .host_visible = info.host_visible,
        });

        // Create image view
        auto image_view = render_device_->create_image_view({
            .image = image,
            .type = to_image_view_type(info.type),
            .format = info.format,
        });

        // Create sampler
        auto sampler = render_device_->create_sampler({
            .filter = info.filter,
            .address_mode_u = info.u,
            .address_mode_v = info.v,
            .address_mode_w = info.w,
            .mip_lod_bias = 0.f,   // TODO: Make this configurable
            .max_anisotropy = 0.f, // TODO: Make this configurable
            .compare_func = CompareFunc::Always,
            .min_lod = 0.f, // TODO: Make this configurable
            .max_lod = 0.f, // TODO: Make this configurable
        });

        if (info.host_visible) {
            render_context_.memcpy(image, bytes);
        } else {
            render_context_.copy_image_staging({
                .bytes = bytes,
                .dst = image,
                .dst_initial_layout = ImageLayout::Undefined,
                .dst_final_layout = ImageLayout::ShaderReadOnly,
                .dst_offset = 0,
                .dst_size = {info.width, info.height, 1},
            });
        }

        const auto id = static_cast<texture_id_t>(textures_.size());
        auto& texture = textures_.emplace_back(std::piecewise_construct,
                                               std::forward_as_tuple(id),
                                               std::forward_as_tuple(render_device_->to_unique(image),
                                                                     render_device_->to_unique(image_view),
                                                                     render_device_->to_unique(sampler),
                                                                     info));
        return std::make_pair(id, &texture.second);
    }

    Mesh* Renderer::find_mesh(mesh_id_t mesh_id)
    {
        if (auto iter = std::ranges::find_if(meshes_, find_by_id(mesh_id)); iter != meshes_.end()) {
            return &(iter->second);
        }
        return nullptr;
    }

    Texture* Renderer::find_texture(texture_id_t texture_id)
    {
        if (auto iter = std::ranges::find_if(textures_, find_by_id(texture_id)); iter != textures_.end()) {
            return &(iter->second);
        }
        return nullptr;
    }

    Material* Renderer::find_material(material_id_t material_id)
    {
        if (auto iter = std::ranges::find_if(materials_, find_by_id(material_id)); iter != materials_.end()) {
            return &(iter->second);
        }
        return nullptr;
    }

    void Renderer::create_default_textures()
    {
        // Create 1x1 white texture
        {
            constexpr auto tex_white_info = TextureInfo{
                .width = 1,
                .height = 1,
                .type = TextureType::Tex2D,
                .format = Format::B8G8R8A8_Srgb,
                .u = AddressMode::Repeat,
                .v = AddressMode::Repeat,
                .w = AddressMode::Repeat,
                .filter = Filter::Linear,
                .host_visible = false,
            };
            constexpr auto tex_white_dat = std::array{0xFFFFFFFF};
            static_assert(sizeof(tex_white_dat) == 4);
            create_texture(tex_white_info, std::as_bytes(std::span{tex_white_dat}));
        }
    }
} // namespace orion
