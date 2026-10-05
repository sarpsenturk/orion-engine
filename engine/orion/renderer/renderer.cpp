#include "renderer.hpp"

#include "orion/core/window.hpp"
#include "orion/lib/array.hpp"

#include <imgui.h>

#include <fstream>
#include <stdexcept>
#include <vector>

namespace orion
{
    static std::vector<uint32_t> load_shader(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + path);
        }

        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint32_t> spirv(size / sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(spirv.data()), size);

        return spirv;
    }

    Renderer::Renderer(const RendererDesc& desc)
        : m_device(gpu::create_device({}))
        , m_swapchain(gpu::create_swapchain(
              m_device,
              gpu::SwapchainDesc{
                  .native_window_handle = desc.window->handle(),
                  .width                = static_cast<std::uint32_t>(desc.window->width()),
                  .height               = static_cast<std::uint32_t>(desc.window->height()),
                  .image_count          = 3u,
                  .image_format         = gpu::Format::BGRA8_Unorm,
              }))
        , m_pso(gpu::create_graphics_pipeline(
              m_device,
              gpu::GraphicsPipelineDesc{
                  .spirv      = load_shader(ORION_SHADER_OUTPUT_DIR "/mesh.spv"),
                  .topology   = gpu::Topology::TriangleList,
                  .rasterizer = {
                      .cull_mode      = gpu::CullMode::Back,
                      .front_face_ccw = true,
                  },
                  .depth_stencil = {
                      .depth_test_enable  = true,
                      .depth_write_enable = true,
                      .depth_compare_op   = gpu::CompareOp::Less,
                  },
                  .color_attachments = {{gpu::Format::BGRA8_Unorm}},
                  .depth_attachment  = gpu::Format::D32_Float,
              }))
        , m_depth_image(gpu::create_image(m_device, gpu::ImageDesc{
                                                        .width  = static_cast<std::uint32_t>(desc.window->width()),
                                                        .height = static_cast<std::uint32_t>(desc.window->height()),
                                                        .format = gpu::Format::D32_Float,
                                                        .usage  = gpu::ImageUsage::DepthStencilAttachment,
                                                    }))
        , m_depth_image_view(gpu::create_image_view(m_device, {m_depth_image}))
        , m_vertex_buffer(gpu::create_buffer(m_device, gpu::BufferDesc{
                                                           .size   = sizeof(Vertex) * max_vertex_count,
                                                           .memory = gpu::Memory::Default,
                                                           .usage  = gpu::BufferUsage::DeviceAddress,
                                                       }))
        , m_index_buffer(gpu::create_buffer(m_device, gpu::BufferDesc{
                                                          .size   = sizeof(std::uint32_t) * max_index_count,
                                                          .memory = gpu::Memory::Default,
                                                          .usage  = gpu::BufferUsage::IndexBuffer,
                                                      }))
        , m_scene_buffer(gpu::create_buffer(m_device, gpu::BufferDesc{
                                                          .size   = sizeof(SceneData) * frames_in_flight,
                                                          .memory = gpu::Memory::Default,
                                                          .usage  = gpu::BufferUsage::DeviceAddress,
                                                      }))
        , m_material_buffer(gpu::create_buffer(m_device, gpu::BufferDesc{
                                                             .size   = sizeof(MaterialData) * max_material_count,
                                                             .memory = gpu::Memory::Default,
                                                             .usage  = gpu::BufferUsage::DeviceAddress,
                                                         }))
        , m_frame_data(make_array<frames_in_flight>([this]() {
            return FrameData{
                .command_allocator = gpu::create_command_allocator(m_device, gpu::Queue::Graphics),
                .delete_queue      = DeletionQueue{m_device},
            };
        }))
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding              = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplOrion_Init({
            .native_window_handle     = desc.window->handle(),
            .device                   = m_device,
            .image_count              = 3u,
            .color_attachment_formats = {{gpu::Format::BGRA8_Unorm}},
        });

        // Load Fonts
        // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
        //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
        // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //style.FontSizeBase = 20.0f;
        //io.Fonts->AddFontDefaultVector();
        //io.Fonts->AddFontDefaultBitmap();
        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
        //IM_ASSERT(font != nullptr);
    }

    Renderer::~Renderer()
    {
        // Wait until the GPU is idle before tearing down Renderer
        gpu::wait_idle(m_device);

        // ImGui cleanup
        ImGui_ImplOrion_Shutdown();
        ImGui::DestroyContext();

        for (auto& frame_data : m_frame_data) {
            frame_data.delete_queue.flush();
            gpu::destroy_command_allocator(m_device, frame_data.command_allocator);
        }
        gpu::destroy_buffer(m_device, m_material_buffer);
        gpu::destroy_buffer(m_device, m_scene_buffer);
        gpu::destroy_buffer(m_device, m_index_buffer);
        gpu::destroy_buffer(m_device, m_vertex_buffer);
        gpu::destroy_image(m_device, m_depth_image);
        gpu::destroy_image_view(m_device, m_depth_image_view);
        gpu::destroy_pipeline_state(m_device, m_pso);
        gpu::destroy_swapchain(m_device, m_swapchain);
        gpu::destroy_device(m_device);
    }

    bool Renderer::begin_frame()
    {
        auto& frame_data = current_frame_data();

        // Wait until the previous submission for this frame has completed
        gpu::wait_timeline(m_device, frame_data.timeline_point);

        // Flush the deletion queue for this frame
        frame_data.delete_queue.flush();

        // Reset command allocator
        gpu::reset_command_allocator(m_device, frame_data.command_allocator);

        // Start the Dear ImGui frame
        ImGui_ImplOrion_NewFrame();
        ImGui::NewFrame();

        // If the swapchain is out of date, we need to resize it before acquiring the next frame
        if (!m_swapchain_out_of_date) {
            // Acquire the next swapchain frame
            if (auto result = gpu::acquire_swapchain_frame(m_device, m_swapchain)) {
                frame_data.swapchain_frame = *result;
                return true;
            } else {
                m_swapchain_out_of_date = true;
            }
        }
        // ImGui::Render() won't be called if the swapchain is out of date, so we need to call ImGui::EndFrame()
        ImGui::EndFrame();
        return false;
    }

    void Renderer::render(const View& view)
    {
        auto& frame_data = current_frame_data();

        // Update scene data
        SceneData scene_data;
        scene_data.view_projection = view.camera.projection * view.camera.view;
        gpu::copy_to_buffer(
            m_device,
            m_scene_buffer,
            current_frame_index() * sizeof(SceneData),
            std::as_bytes(std::span{&scene_data, 1}));

        // Begin recording commands for this frame
        auto* command_buffer = gpu::begin_command_buffer(m_device, frame_data.command_allocator);

        // Transition the swapchain image to a color attachment
        // Transition the depth image to a depth attachment
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image      = frame_data.swapchain_frame.image,
                        .src_stages = gpu::PipelineStage::ColorAttachmentOutput,
                        .src_access = gpu::Access::None,
                        .dst_stages = gpu::PipelineStage::ColorAttachmentOutput,
                        .dst_access = gpu::Access::ColorAttachmentWrite,
                        .old_layout = gpu::ImageLayout::Undefined,
                        .new_layout = gpu::ImageLayout::ColorAttachment,
                    },
                    gpu::ImageBarrier{
                        .image      = m_depth_image,
                        .src_stages = gpu::PipelineStage::None,
                        .src_access = gpu::Access::None,
                        .dst_stages = gpu::PipelineStage::DepthStencilTests,
                        .dst_access = gpu::Access::DepthStencilAttachmentWrite,
                        .old_layout = gpu::ImageLayout::Undefined,
                        .new_layout = gpu::ImageLayout::DepthStencilAttachment,
                    },
                }},
            });

        // Bind graphics pipeline state (PSO)
        gpu::bind_pso(command_buffer, m_pso);

        // Bind index buffer
        gpu::bind_index_buffer(command_buffer, m_index_buffer);

        // Push buffer addresses to the shader via push constants
        const auto push_constants = PushConstants{
            .vertex_buffer_address   = gpu::get_buffer_device_address(m_vertex_buffer),
            .scene_buffer_address    = gpu::get_buffer_device_address(m_scene_buffer) +
                                       current_frame_index() * sizeof(SceneData),
            .material_buffer_address = gpu::get_buffer_device_address(m_material_buffer),
        };
        gpu::push_constants(command_buffer, 0u, std::as_bytes(std::span{&push_constants, 1}));

        // Get swapchain image extent
        const auto swapchain_extent = gpu::get_image_extent_2d(frame_data.swapchain_frame.image);

        // Set viewport and scissor
        gpu::set_viewport(
            command_buffer,
            gpu::Viewport{
                .x         = 0.0f,
                .y         = 0.0f,
                .width     = static_cast<float>(swapchain_extent.width),
                .height    = static_cast<float>(swapchain_extent.height),
                .min_depth = 0.0f,
                .max_depth = 1.0f,
            });
        gpu::set_scissor(command_buffer, gpu::Rect2D{.extent = swapchain_extent});

        // Begin render pass
        gpu::begin_render_pass(
            command_buffer,
            gpu::RenderPass{
                .color_attachments = {{
                    gpu::RenderPassAttachment{
                        .image_view  = frame_data.swapchain_frame.image_view,
                        .load_op     = gpu::LoadOp::Clear,
                        .clear_value = gpu::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f},
                    },
                }},
                .depth_attachment  = gpu::RenderPassAttachment{
                    .image_view  = m_depth_image_view,
                    .load_op     = gpu::LoadOp::Clear,
                    .clear_value = gpu::ClearValue{.depth_stencil = {1.0f, 0u}},
                },
            });

        // Draw objects
        for (const auto& object : view.objects) {
            // Update material index
            const auto material = SlotMap<MaterialData>::Handle::from_uint64(object.material.handle).index;
            gpu::push_constants(command_buffer, offsetof(PushConstants, material_index), std::as_bytes(std::span{&material, 1}));

            // Update model matrix
            gpu::push_constants(command_buffer, offsetof(PushConstants, model), std::as_bytes(std::span{&object.model, 1}));

            // Draw mesh
            const auto* mesh_data = m_meshes.get(SlotMap<MeshData>::Handle::from_uint64(object.mesh.handle));
            gpu::draw_indexed(
                command_buffer,
                gpu::DrawIndexed{
                    .index_count    = mesh_data->index_count,
                    .instance_count = 1u,
                    .first_index    = mesh_data->index_offset,
                    .vertex_offset  = mesh_data->vertex_offset,
                    .first_instance = 0u,
                });
        }

        // End render pass
        gpu::end_render_pass(command_buffer);

        // Begin Dear ImGui render pass
        gpu::begin_render_pass(
            command_buffer,
            gpu::RenderPass{
                .color_attachments = {{
                    gpu::RenderPassAttachment{
                        .image_view = frame_data.swapchain_frame.image_view,
                        .load_op    = gpu::LoadOp::Load,
                    },
                }},
            });

        // Render ImGui draw data
        ImGui::Render();
        auto* imgui_draw_data = ImGui::GetDrawData();
        ImGui_ImplOrion_RenderDrawData(imgui_draw_data, command_buffer);

        // End Dear ImGui render pass
        gpu::end_render_pass(command_buffer);

        // Transition the swapchain image to a presentable layout
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image      = frame_data.swapchain_frame.image,
                        .src_stages = gpu::PipelineStage::ColorAttachmentOutput,
                        .src_access = gpu::Access::ColorAttachmentWrite,
                        .dst_stages = gpu::PipelineStage::None,
                        .dst_access = gpu::Access::None,
                        .old_layout = gpu::ImageLayout::ColorAttachment,
                        .new_layout = gpu::ImageLayout::Present,
                    },
                }},
            });

        // Submit commands for execution
        frame_data.timeline_point = gpu::submit(
            m_device,
            gpu::Queue::Graphics,
            gpu::Submit{
                .command_buffers = {{command_buffer}},
                .swapchain       = m_swapchain,
                .swapchain_frame = &frame_data.swapchain_frame,
            });
    }

    void Renderer::end_frame()
    {
        auto& frame_data = current_frame_data();

        // Present the swapchain frame
        gpu::present(m_device, m_swapchain, frame_data.swapchain_frame);

        // Increment the frame counter for the next frame
        ++m_frame_counter;
    }

    Mesh Renderer::create_mesh(
        std::span<const Vertex> vertices,
        std::span<const std::uint32_t> indices)
    {
        if (m_vertex_offset + vertices.size() > max_vertex_count ||
            m_index_offset + indices.size() > max_index_count) {
            throw std::runtime_error("Exceeded Renderer mesh limits.");
        } else {
            const auto vertex_offset = m_vertex_offset;
            const auto index_offset  = m_index_offset;

            gpu::copy_to_buffer(
                m_device,
                m_vertex_buffer,
                vertex_offset * sizeof(Vertex),
                std::as_bytes(vertices));
            gpu::copy_to_buffer(
                m_device,
                m_index_buffer,
                index_offset * sizeof(std::uint32_t),
                std::as_bytes(indices));

            m_vertex_offset += static_cast<std::uint32_t>(vertices.size());
            m_index_offset += static_cast<std::uint32_t>(indices.size());

            const auto handle = m_meshes.emplace(MeshData{
                .vertex_count  = static_cast<std::uint32_t>(vertices.size()),
                .vertex_offset = vertex_offset,
                .index_offset  = index_offset,
                .index_count   = static_cast<std::uint32_t>(indices.size()),
            });
            return Mesh{.handle = handle.to_uint64()};
        }
    }

    void Renderer::destroy_mesh(Mesh mesh)
    {
        const auto handle = SlotMap<MeshData>::Handle::from_uint64(mesh.handle);
        m_meshes.erase(handle);
    }

    Texture Renderer::create_texture(const TextureDesc& desc, std::span<const std::byte> data)
    {
        // Create the texture image
        auto* image = gpu::create_image(
            m_device,
            gpu::ImageDesc{
                .width        = desc.width,
                .height       = desc.height,
                .depth        = desc.depth,
                .format       = gpu::Format::RGBA8_Unorm,
                .mip_levels   = 1u,
                .array_layers = 1u,
                .usage        = gpu::ImageUsage::TransferDst | gpu::ImageUsage::Sampled,
            });

        // Copy texture data into buffer
        auto* staging_buffer = gpu::create_buffer(
            m_device,
            gpu::BufferDesc{
                .size   = data.size(),
                .memory = gpu::Memory::Upload,
                .usage  = gpu::BufferUsage::TransferSrc,
            });
        gpu::copy_to_buffer(m_device, staging_buffer, 0ull, data);

        // Copy texture data from the buffer to the image
        auto* command_allocator = gpu::create_command_allocator(m_device, gpu::Queue::Graphics);
        auto* command_buffer    = gpu::begin_command_buffer(m_device, command_allocator);
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image      = image,
                        .src_stages = gpu::PipelineStage::None,
                        .src_access = gpu::Access::None,
                        .dst_stages = gpu::PipelineStage::Transfer,
                        .dst_access = gpu::Access::TransferWrite,
                        .old_layout = gpu::ImageLayout::Undefined,
                        .new_layout = gpu::ImageLayout::TransferDst,
                    },
                }},
            });
        gpu::copy_buffer_to_image(
            command_buffer,
            staging_buffer,
            image,
            gpu::BufferImageCopy{
                .buffer_row_length   = desc.width,
                .buffer_image_height = desc.height,
                .image_extent        = gpu::Extent3D{
                    .width  = desc.width,
                    .height = desc.height,
                    .depth  = desc.depth,
                },
            });
        gpu::barrier(
            command_buffer,
            gpu::Barrier{
                .image_barriers = {{
                    gpu::ImageBarrier{
                        .image      = image,
                        .src_stages = gpu::PipelineStage::Transfer,
                        .src_access = gpu::Access::TransferWrite,
                        .dst_stages = gpu::PipelineStage::FragmentShader,
                        .dst_access = gpu::Access::ShaderRead,
                        .old_layout = gpu::ImageLayout::TransferDst,
                        .new_layout = gpu::ImageLayout::ShaderReadOnly,
                    },
                }},
            });
        gpu::wait_timeline(
            m_device,
            gpu::submit(
                m_device,
                gpu::Queue::Graphics,
                gpu::Submit{
                    .command_buffers = {{command_buffer}},
                }));

        // Create image view
        auto* image_view = gpu::create_image_view(
            m_device,
            gpu::ImageViewDesc{
                .image = image,
            });

        // Cleanup staging buffer and command allocator
        gpu::destroy_command_allocator(m_device, command_allocator);
        gpu::destroy_buffer(m_device, staging_buffer);

        const auto handle = m_textures.emplace(image, image_view);
        // Update descriptor heap
        gpu::set_shader_resource(m_device, image_view, handle.index);

        return Texture{.handle = handle.to_uint64()};
    }

    void Renderer::destroy_texture(Texture texture)
    {
        const auto handle = SlotMap<TextureData>::Handle::from_uint64(texture.handle);
        if (m_textures.is_valid(handle)) {
            const auto& texture_data = *m_textures.get(handle);
            queue_delete(texture_data.image_view);
            queue_delete(texture_data.image);
            m_textures.erase(handle);
        }
    }

    Material Renderer::create_material(const MaterialDesc& desc)
    {
        const auto albedo_texture = SlotMap<TextureData>::Handle::from_uint64(desc.albedo.handle);
        const auto material       = m_materials.emplace(albedo_texture.index);
        if (material.index >= max_material_count) {
            m_materials.erase(material);
            throw std::runtime_error("Exceeded Renderer material limits.");
        }
        const auto* material_data = m_materials.get(material);
        gpu::copy_to_buffer(
            m_device,
            m_material_buffer,
            sizeof(MaterialData) * material.index,
            std::as_bytes(std::span{material_data, 1}));
        return Material{.handle = material.to_uint64()};
    }

    void Renderer::destroy_material(Material material)
    {
        const auto handle = SlotMap<MaterialData>::Handle::from_uint64(material.handle);
        m_materials.erase(handle);
    }

    void Renderer::resize_swapchain(std::uint32_t width, std::uint32_t height)
    {
        // Wait until the GPU is so we can safely resize the swapchain
        gpu::wait_idle(m_device);
        gpu::resize_swapchain(m_device, m_swapchain, width, height);
        m_swapchain_out_of_date = false;
    }
} // namespace orion
