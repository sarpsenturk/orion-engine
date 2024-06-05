#pragma once

#include "orion-platform/window.h"

#include "orion-math/vector/vector2.h"

#include <imgui.h>

namespace orion
{
    class RenderDevice;
    class RenderContext;
} // namespace orion

struct ImGui_ImplOrion_Desc {
    orion::RenderDevice* device;
    orion::RenderContext* context;
    orion::Vector2_u display_size;
};

void ImGui_ImplOrion_Init(const ImGui_ImplOrion_Desc& desc);
void ImGui_ImplOrion_Shutdown();
void ImGui_ImplOrion_NewFrame();
void ImGui_ImplOrion_RenderDrawData(ImDrawData* draw_data);
void ImGui_ImplOrion_OnEvent(const orion::WindowEvent& event);
