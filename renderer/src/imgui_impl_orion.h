#pragma once

#include "orion-platform/window.h"

#include "orion-math/vector/vector2.h"

#include <imgui.h>

namespace orion
{
    class RenderDevice;
    class TransferContext;
    class CommandList;
} // namespace orion

struct ImGui_ImplOrion_Desc {
    orion::RenderDevice* device;
    orion::TransferContext* transfer;
    orion::Vector2_u display_size;
};

void ImGui_ImplOrion_Init(const ImGui_ImplOrion_Desc& desc);
void ImGui_ImplOrion_Shutdown();
void ImGui_ImplOrion_NewFrame();
void ImGui_ImplOrion_RenderDrawData(ImDrawData* draw_data, orion::RenderDevice* device, orion::CommandList* command_list);
void ImGui_ImplOrion_OnEvent(const orion::WindowEvent& event);
