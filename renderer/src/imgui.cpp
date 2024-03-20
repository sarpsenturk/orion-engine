#include "orion-renderer/imgui.h"

#include "imgui_impl_orion.h"

#include <utility>

namespace orion
{
    ImGuiContext::ImGuiContext(ImGuiContext&& other) noexcept
        : destroy_(std::exchange(other.destroy_, false))
    {
    }

    ImGuiContext::~ImGuiContext()
    {
        if (destroy_) {
            ImGui_ImplOrion_Shutdow();
            ImGui::DestroyContext();
        }
    }
} // namespace orion
