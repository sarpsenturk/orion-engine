#pragma once

#include "orion-core/window.h"

#include "orion-renderapi/render_device.h"

#include <imgui.h>

struct ImGui_ImplOrion_InitDesc {
    orion::Window* window;
    orion::RenderDevice* device;
};

void ImGui_ImplOrion_Init(const ImGui_ImplOrion_InitDesc& desc);
void ImGui_ImplOrion_Shutdow();
void ImGui_ImplOrion_NewFrame();
void ImGui_ImplOrion_RenderDrawData(ImDrawData* draw_data, orion::CommandList& command_list);
