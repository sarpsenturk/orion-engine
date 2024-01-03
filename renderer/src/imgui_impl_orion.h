// dear imgui: Platform Backend for Orion
// This backend handles both platform features (input, window resizing, etc.) and rendering.
// It is easier if you use orion::Renderer class, but it is not required

// Issues:
//  [ ] No viewport support yet.
//  [ ] No input character support as Orion doesn't support those yet.

#pragma once

#include "orion-renderer/shader.h"

#include "orion-core/window.h"

#include "orion-renderapi/render_device.h"

#include <imgui.h>

struct ImGui_ImplOrion_InitDesc {
    orion::Window* window;
    orion::RenderDevice* device;
    orion::CommandAllocator* command_allocator;
    orion::RenderPassHandle render_pass;
    orion::ShaderManager* shader_manager;
};

void ImGui_ImplOrion_Init(const ImGui_ImplOrion_InitDesc& desc);
void ImGui_ImplOrion_Shutdow();
void ImGui_ImplOrion_NewFrame(std::int8_t frame_index);
void ImGui_ImplOrion_RenderDrawData(ImDrawData* draw_data, orion::CommandList* cmd_list);
