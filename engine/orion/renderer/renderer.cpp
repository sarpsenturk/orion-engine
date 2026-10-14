#include "renderer.hpp"

#include "orion/core/window.hpp"
#include "orion/lib/array.hpp"

#include <imgui.h>

#include <fstream>
#include <stdexcept>
#include <vector>

namespace orion
{
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
        , m_depth_image(gpu::create_image(
              m_device,
              gpu::ImageDesc{
                  .width  = static_cast<std::uint32_t>(desc.window->width()),
                  .height = static_cast<std::uint32_t>(desc.window->height()),
                  .format = gpu::Format::D32_Float,
                  .usage  = gpu::ImageUsage::DepthStencilAttachment,
              }))
        , m_depth_image_view(gpu::create_image_view(m_device, {m_depth_image}))
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
        gpu::destroy_image(m_device, m_depth_image);
        gpu::destroy_image_view(m_device, m_depth_image_view);
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

    void Renderer::render(const View& /*view*/)
    {
        auto& frame_data = current_frame_data();

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

        // TODO: Render here

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

    void Renderer::resize_swapchain(std::uint32_t width, std::uint32_t height)
    {
        // Wait until the GPU is so we can safely resize the swapchain
        gpu::wait_idle(m_device);
        gpu::resize_swapchain(m_device, m_swapchain, width, height);
        m_swapchain_out_of_date = false;
    }
} // namespace orion
