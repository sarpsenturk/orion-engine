// dear imgui: Platform Backend for Orion
// This backend handles both platform features (input, window resizing, etc.) and rendering.
// It is easier if you use orion::Renderer class, but it is not required

// Issues:
//  [ ] No viewport support yet.
//  [ ] No input character support as Orion doesn't support those yet.

#pragma once

#include "orion-renderapi/handles.h"

#include "orion-renderer/config.h"

#include <imgui.h>

namespace orion
{
    // Forward declare
    class Window;
    class RenderDevice;
    class CommandAllocator;
    class CommandList;
    class ShaderManager;
} // namespace orion

struct ImGui_ImplOrion_InitDesc {
    orion::Window* window;
    orion::RenderDevice* device;
    orion::CommandAllocator* command_allocator;
    orion::RenderPassHandle render_pass;
    orion::ShaderManager* shader_manager;
};

void ImGui_ImplOrion_Init(const ImGui_ImplOrion_InitDesc& desc);
void ImGui_ImplOrion_Shutdow();
void ImGui_ImplOrion_NewFrame(orion::frame_index_t frame_index);
void ImGui_ImplOrion_RenderDrawData(ImDrawData* draw_data, orion::CommandList* cmd_list);
