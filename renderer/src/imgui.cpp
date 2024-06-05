#include "orion-renderer/imgui.h"

#include "imgui_impl_orion.h"

namespace orion
{
    ImGuiLayer::~ImGuiLayer()
    {
        ImGui_ImplOrion_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::on_draw()
    {
        on_user_draw();
    }

    void ImGuiLayer::on_event(const WindowEvent& event)
    {
        return on_user_event(event);
    }

    void ImGuiLayer::on_user_draw()
    {
    }

    void ImGuiLayer::on_user_event(const WindowEvent& event)
    {
        ImGui_ImplOrion_OnEvent(event);
    }
} // namespace orion
