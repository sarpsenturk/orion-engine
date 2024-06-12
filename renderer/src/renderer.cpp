#include "orion-renderer/renderer.h"

#include "orion-renderer/colors.h"
#include "orion-renderer/frame.h"
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

        RenderPassHandle create_present_render_pass(RenderDevice* device)
        {
            return device->create_render_pass({
                .bind_point = PipelineBindPoint::Graphics,
                .color_attachments = {{
                    AttachmentDesc{
                        .format = Format::B8G8R8A8_Srgb,
                        .load_op = AttachmentLoadOp::DontCare,
                        .store_op = AttachmentStoreOp::Store,
                        .initial_layout = ImageLayout::Undefined,
                        .layout = ImageLayout::ColorAttachment,
                        .final_layout = ImageLayout::PresentSrc,
                    },
                }},
            });
        }

        DescriptorPoolHandle create_material_descriptor_pool(RenderDevice* device)
        {
            // TODO: For more than 1 material we need a dynamic way to handle this.
            //  We can't allocate a pool for UINT16_MAX descriptors with UINT16_MAX sizes each (I assume!??)
            //  Probably a free list approach where we create descriptor pools with N max_descriptors & N count for each DescriptorPoolSize
            //  and add them to a free list as materials are created would work.
            return device->create_descriptor_pool({
                .max_descriptors = 16,
                .flags = {},
                .sizes = {{
                    DescriptorPoolSize{
                        .type = DescriptorType::ConstantBuffer,
                        .count = 16,
                    },
                    DescriptorPoolSize{
                        .type = DescriptorType::SampledImage,
                        .count = 16,
                    },
                    DescriptorPoolSize{
                        .type = DescriptorType::Sampler,
                        .count = 16,
                    },
                }},
            });
        }

        auto find_by_id(auto id)
        {
            return [id](const auto& pair) { return pair.first == id; };
        }

        PerFrame<UniqueDescriptor> create_descriptor_per_frame(RenderDevice* device, DescriptorLayoutHandle layout, DescriptorPoolHandle pool)
        {
            return generate_per_frame([&](frame_index_t) { return device->make_unique<DescriptorHandle_tag>(layout, pool); });
        }
    } // namespace

    Renderer::Renderer(const RendererDesc& desc)
        : render_backend_module_(load_backend_module(desc.backend))
        , render_backend_(create_render_backend(render_backend_module_))
        , render_device_(create_render_device(render_backend_.get()))
        , render_size_(desc.render_size)
        , object_effect_(create_shader_effect("object.vs", "object.ps"))
        , object_pass_(create_shader_pass(&object_effect_))
        , present_effect_(create_shader_effect("present.vs", "present.ps"))
        , present_pass_(create_shader_pass(&present_effect_))
        , present_render_pass_(create_present_render_pass(render_device_.get()))
        , descriptor_pool_(create_material_descriptor_pool(render_device_.get()))
        , frame_data_(generate_per_frame(std::bind_front(std::mem_fn(&Renderer::create_frame_data), this)))
        , scene_descriptors_(create_descriptor_per_frame(render_device_.get(), object_effect_.descriptor_layout(0), descriptor_pool_))
        , object_data_descriptors_(create_descriptor_per_frame(render_device_.get(), object_effect_.descriptor_layout(2), descriptor_pool_))
        , scene_cbuffer_(DynamicGPUBuffer::create(render_device_.get(), sizeof(SceneCBuffer), GPUBufferUsageFlags::ConstantBuffer))
        , object_buffer_(DynamicGPUBuffer::create(render_device_.get(), sizeof(RenderObjGPUData) * max_render_objects, GPUBufferUsageFlags::StorageBuffer))
        , present_sampler_(create_present_sampler(render_device_.get()))
    {
        for (frame_index_t index = 0; index < frames_in_flight; ++index) {
            render_device_->write_descriptor(scene_descriptors_[index].get(), 0, DescriptorType::ConstantBuffer, scene_cbuffer_.descriptor_desc(index));
            render_device_->write_descriptor(object_data_descriptors_[index].get(), 0, DescriptorType::StorageBuffer, object_buffer_.descriptor_desc(index));

            const auto& frame_data = frame_data_[index];
            render_device_->write_descriptor(frame_data.render_output_descriptor.get(), 0, frame_data.render_target.image_view(), ImageLayout::ShaderReadOnly);
            render_device_->write_descriptor(frame_data.render_output_descriptor.get(), 1, present_sampler_);
        }
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

        const auto obj_data = std::array{RenderObjGPUData{.transform = *obj.transform}};
        object_buffer_.update(render_device_.get(), std::as_bytes(std::span{obj_data}), sizeof(RenderObjGPUData) * object_index);
    }

    void Renderer::render(const Camera& camera)
    {
        auto& frame = current_frame();

        render_device_->wait_for_fence(frame.frame_fence.get());

        if (frame_counter_ % frames_in_flight == 0) {
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

        const auto scene_data = std::array{SceneCBuffer{.view_projection = camera.view_projection()}};
        scene_cbuffer_.update(render_device_.get(), std::as_bytes(std::span{scene_data}));
        render_command->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = object_effect_.pipeline_layout(),
            .index = descriptor_index_frame,
            .descriptor = scene_descriptors_[scene_cbuffer_.index()].get(),
        });

        render_command->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = object_effect_.pipeline_layout(),
            .index = descriptor_index_object,
            .descriptor = object_data_descriptors_[object_buffer_.index()].get(),
        });

        for (std::size_t index = 0; const auto& obj : objects_) {
            const auto* material = find_material(obj.material);
            render_command->bind_pipeline({.pipeline = object_pass_.pipeline(), .bind_point = PipelineBindPoint::Graphics});
            render_command->bind_descriptor({
                .bind_point = PipelineBindPoint::Graphics,
                .pipeline_layout = object_effect_.pipeline_layout(),
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
            ImGui_ImplOrion_RenderDrawData(ImGui::GetDrawData(), render_device_.get(), render_command);
        }

        render_command->end_render_pass();
        render_command->end();

        const auto render_semaphore = frame.render_semaphore.get();
        const auto submit = SubmitDesc{
            .queue_type = CommandQueueType::Graphics,
            .command_lists = {&render_command, 1},
            .signal_semaphores = {&render_semaphore, 1},
        };
        render_device_->submit(submit, FenceHandle::invalid());
    }

    Renderer::FrameData Renderer::create_frame_data(frame_index_t)
    {
        auto command_allocator = render_device_->create_command_allocator({.queue_type = CommandQueueType::Graphics, .reset_command_buffer = false});
        auto render_command = command_allocator->create_command_list();
        auto present_command = command_allocator->create_command_list();

        const auto render_target_desc = RenderTargetDesc{
            .size = render_size_,
            .image_usage = ImageUsageFlags::ColorAttachment | ImageUsageFlags::SampledImage,
            .initial_layout = ImageLayout::Undefined,
            .final_layout = ImageLayout::ShaderReadOnly,
        };

        return {
            .command_allocator = std::move(command_allocator),
            .render_command = std::move(render_command),
            .present_command = std::move(present_command),
            .render_target = RenderTarget::create(render_device_.get(), render_target_desc),
            .render_output_descriptor = render_device_->make_unique<DescriptorHandle_tag>(present_effect_.descriptor_layout(0), descriptor_pool_),
            .frame_fence = render_device_->make_unique<FenceHandle_tag>(FenceDesc{.start_finished = true}),
            .render_semaphore = render_device_->make_unique<SemaphoreHandle_tag>(),
            .swapchain_image_semaphore = render_device_->make_unique<SemaphoreHandle_tag>(),
            .present_semaphore = render_device_->make_unique<SemaphoreHandle_tag>(),
            .staging_buffer = render_device_->make_unique<GPUBufferHandle_tag>(GPUBufferDesc{
                .size = staging_buffer_size,
                .usage = GPUBufferUsageFlags::TransferSrc,
                .host_visible = true,
            }),
        };
    }

    void Renderer::imgui_init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;
        auto transfer = transfer_context();
        ImGui_ImplOrion_Init({render_device_.get(), &transfer, render_size_});
    }

    SwapchainHandle Renderer::create_swapchain(const Window& window)
    {
        ORION_ASSERT(swapchain_.get() == SwapchainHandle::invalid() && "Only 1 swapchain is allowed");
        const auto desc = SwapchainDesc{
            .image_count = frames_in_flight,
            .image_format = Format::B8G8R8A8_Srgb,
            .image_size = window.size(),
            .image_usage = ImageUsageFlags::ColorAttachment,
            .vsync = true,
        };
        swapchain_ = render_device_->make_unique<SwapchainHandle_tag>(window, desc);
        const auto framebuffer_count = render_device_->create_framebuffers_for_swapchain(swapchain_.get(), present_render_pass_, swapchain_framebuffers_);
        ORION_ASSERT(framebuffer_count == frames_in_flight);
        return swapchain_.get();
    }

    void Renderer::present()
    {
        auto& frame = current_frame();
        auto* present_command = frame.present_command.get();

        // Acquire swapchain image
        const auto swapchain_image_semaphore = frame.swapchain_image_semaphore.get();
        const auto image_index = render_device_->acquire_swapchain_image(swapchain_.get(), swapchain_image_semaphore);

        // Begin a new render pass to render the internal output image to the back buffer
        present_command->begin();
        present_command->begin_render_pass({
            .render_pass = present_render_pass_,
            .framebuffer = swapchain_framebuffers_[image_index],
            .render_area = {
                .offset = {},
                .size = render_size_, // TODO: Swapchain size and internal render size might be different
            },
            .clear_color = {},
        });

        present_command->bind_pipeline({.pipeline = present_pass_.pipeline(), .bind_point = PipelineBindPoint::Graphics});
        present_command->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = present_effect_.pipeline_layout(),
            .index = 0,
            .descriptor = frame.render_output_descriptor.get(),
        });
        present_command->set_viewports(Viewport{
            .position = {0.f, 0.f},
            .size = vector_cast<float>(render_size_), // TODO: Swapchain size and internal render size might be different
            .depth = {0.f, 1.f},
        });
        present_command->set_scissors(Scissor{
            .offset = {0, 0},
            .size = render_size_, // TODO: Swapchain size and internal render size might be different
        });
        present_command->draw({.vertex_count = 3, .instance_count = 1, .first_vertex = 0, .first_instance = 0});
        present_command->end_render_pass();
        present_command->end();

        // Submit presentation commands
        const auto wait_semaphores = std::array{swapchain_image_semaphore, frame.render_semaphore.get()};
        const auto present_semaphore = std::array{frame.present_semaphore.get()};
        const auto submit = SubmitDesc{
            .queue_type = CommandQueueType::Graphics,
            .wait_semaphores = wait_semaphores,
            .command_lists = {&present_command, 1},
            .signal_semaphores = present_semaphore,
        };
        render_device_->submit(submit, frame.frame_fence.get());

        // Swap back buffers / present
        render_device_->swapchain_present(swapchain_.get(), present_semaphore);

        // Move to next frame index
        advance_frame();
    }

    ShaderEffect Renderer::create_shader_effect(const FilePath& vs_path, const FilePath& ps_path)
    {
        const auto base_path = FilePath{render_device_->shader_base_path()};
        return ShaderEffect::create(render_device_.get(), base_path / vs_path, base_path / ps_path);
    }

    ShaderPass Renderer::create_shader_pass(const ShaderEffect* effect, const ShaderPassDesc& desc)
    {
        return ShaderPass::create(render_device_.get(), effect, desc);
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

        transfer_context().copy_buffer_staging({{
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

    std::pair<material_id_t, Material*> Renderer::create_material(const MaterialData& data)
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
            transfer_context().copy_buffer_staging({.bytes = bytes, .dst = constant_buffer});
        }

        auto descriptor = render_device_->create_descriptor(object_effect_.descriptor_layout(1), descriptor_pool_);
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
                                                 std::forward_as_tuple(render_device_->to_unique(constant_buffer),
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
            transfer_context().memcpy(image, bytes);
        } else {
            transfer_context().copy_image_staging({
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

    void Renderer::advance_frame()
    {
        ++frame_counter_;
    }

    TransferContext Renderer::transfer_context()
    {
        auto& frame = current_frame();
        return {render_device_.get(), frame.command_allocator.get(), frame.staging_buffer.get()};
    }
} // namespace orion
