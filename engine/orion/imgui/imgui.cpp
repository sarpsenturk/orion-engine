#include "imgui.hpp"

#include "orion/core/window.hpp"
#include "orion/renderer/renderer.hpp"

#include <stdexcept>

namespace orion
{
    ImGuiContext::ImGuiContext(
        const class Renderer& renderer,
        const class Window& window)
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

        const auto init_info = ImGui_ImplOrion_InitInfo{
            .native_window_handle     = window.handle(),
            .device                   = renderer.device(),
            .image_count              = renderer.swapchain_image_count,
            .color_attachment_formats = {{renderer.swapchain_image_format}},
        };
        if (!ImGui_ImplOrion_Init(init_info)) {
            ImGui::DestroyContext();
            throw std::runtime_error("Failed to initialize ImGui renderer");
        }
    }

    ImGuiContext::~ImGuiContext()
    {
        ImGui_ImplOrion_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiContext::begin_frame()
    {
        ImGui_ImplOrion_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiContext::end_frame()
    {
        ImGui::Render();
    }

    void ImGuiContext::update_platform_windows()
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    bool ImGuiContext::wants_capture_mouse() const
    {
        return ImGui::GetIO().WantCaptureMouse;
    }

    bool ImGuiContext::wants_capture_keyboard() const
    {
        return ImGui::GetIO().WantCaptureKeyboard;
    }
} // namespace orion
