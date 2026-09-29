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
                  .spirv             = load_shader(ORION_SHADER_OUTPUT_DIR "/mesh.spv"),
                  .topology          = gpu::Topology::TriangleList,
                  .cull_mode         = gpu::CullMode::Back,
                  .front_face_ccw    = true,
                  .color_attachments = {{gpu::Format::BGRA8_Unorm}},
              }))
        , m_vertex_buffer(gpu::create_buffer(
              m_device,
              gpu::BufferDesc{
                  .size   = sizeof(Vertex) * max_vertex_count,
                  .memory = gpu::Memory::Default,
                  .usage  = gpu::BufferUsage::DeviceAddress,
              }))
        , m_index_buffer(gpu::create_buffer(
              m_device,
              gpu::BufferDesc{
                  .size   = sizeof(std::uint32_t) * max_index_count,
                  .memory = gpu::Memory::Default,
                  .usage  = gpu::BufferUsage::IndexBuffer,
              }))
        , m_scene_buffer(gpu::create_buffer(
              m_device,
              gpu::BufferDesc{
                  .size   = sizeof(SceneData) * frames_in_flight,
                  .memory = gpu::Memory::Default,
                  .usage  = gpu::BufferUsage::DeviceAddress,
              }))
        , m_frame_data(make_array<frames_in_flight>([this]() {
            return FrameData{
                .command_allocator = gpu::create_command_allocator(m_device, gpu::Queue::Graphics),
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

        for (const auto& frame_data : m_frame_data) {
            gpu::destroy_command_allocator(m_device, frame_data.command_allocator);
        }
        gpu::destroy_buffer(m_device, m_scene_buffer);
        gpu::destroy_buffer(m_device, m_index_buffer);
        gpu::destroy_buffer(m_device, m_vertex_buffer);
        gpu::destroy_pipeline_state(m_device, m_pso);
        gpu::destroy_swapchain(m_device, m_swapchain);
        gpu::destroy_device(m_device);
    }

    bool Renderer::begin_frame()
    {
        auto& frame_data = current_frame_data();

        // Wait until the previous submission for this frame has completed
        gpu::wait_timeline(m_device, frame_data.timeline_point);

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
                }},
            });

        // Bind graphics pipeline state (PSO)
        gpu::bind_pso(command_buffer, m_pso);

        // Bind index buffer
        gpu::bind_index_buffer(command_buffer, m_index_buffer);

        // Push buffer addresses to the shader via push constants
        const auto push_constants = PushConstants{
            .vertex_buffer_address = gpu::get_buffer_device_address(m_vertex_buffer),
            .scene_buffer_address  = gpu::get_buffer_device_address(m_scene_buffer),
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
                        .image_view = frame_data.swapchain_frame.image_view,
                        .load_op    = gpu::LoadOp::Clear,
                    },
                }},
            });

        // Draw objects
        for (const auto& object : view.objects) {
            // Update model matrix
            gpu::push_constants(command_buffer, offsetof(PushConstants, model), std::as_bytes(std::span{&object.model, 1}));

            // Draw mesh
            gpu::draw_indexed(
                command_buffer,
                gpu::DrawIndexed{
                    .index_count    = object.mesh.index_count,
                    .instance_count = 1u,
                    .first_index    = object.mesh.index_offset,
                    .vertex_offset  = object.mesh.vertex_offset,
                    .first_instance = 0u,
                });
        }

        // Render ImGui draw data
        ImGui::Render();
        auto* imgui_draw_data = ImGui::GetDrawData();
        ImGui_ImplOrion_RenderDrawData(imgui_draw_data, command_buffer);

        // End render pass
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

            return Mesh{
                .vertex_count  = static_cast<std::uint32_t>(vertices.size()),
                .vertex_offset = vertex_offset,
                .index_offset  = index_offset,
                .index_count   = static_cast<std::uint32_t>(indices.size()),
            };
        }
    }

    void Renderer::destroy_mesh(Mesh /*mesh*/)
    {
        // No-op for now.
    }

    void Renderer::resize_swapchain(std::uint32_t width, std::uint32_t height)
    {
        // Wait until the GPU is so we can safely resize the swapchain
        gpu::wait_idle(m_device);
        gpu::resize_swapchain(m_device, m_swapchain, width, height);
        m_swapchain_out_of_date = false;
    }
} // namespace orion
