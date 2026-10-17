#include "imgui_renderer.hpp"

#include "orion/core/window.hpp"
#include "orion/renderer/render_graph/graph.hpp"

#include <imgui.h>

#include <stdexcept>

namespace orion
{
    ImGuiRenderer::ImGuiRenderer(const ImGuiRendererDesc& desc)
        : m_device(desc.device)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();
        auto& style                       = ImGui::GetStyle();
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;

        if (!ImGui_ImplOrion_Init({
                .native_window_handle     = desc.window->handle(),
                .device                   = m_device,
                .image_count              = desc.image_count,
                .color_attachment_formats = {{desc.target_format}},
            })) {
            ImGui::DestroyContext();
            throw std::runtime_error("Failed to initialize ImGui renderer");
        }
    }

    ImGuiRenderer::~ImGuiRenderer()
    {
        gpu::wait_idle(m_device);
        ImGui_ImplOrion_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiRenderer::begin_frame()
    {
        ImGui_ImplOrion_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiRenderer::end_frame()
    {
        ImGui::Render();
    }

    void ImGuiRenderer::render_platform_windows()
    {
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    RGTexture ImGuiRenderer::add_pass(
        RenderGraph& graph,
        RGTexture target,
        gpu::LoadOp load_op,
        gpu::ClearColorValue clear_color)
    {
        return graph.add_pass(
            "ImGui", RenderPassType::Graphics,
            [&](RenderPassBuilder& builder) {
                return builder.add_color_attachment(target, load_op, gpu::StoreOp::Store, clear_color);
            },
            [](RenderPassContext& context, RGTexture /*target*/) {
                ImGui_ImplOrion_RenderDrawData(ImGui::GetDrawData(), context.command_buffer());
            });
    }
} // namespace orion
