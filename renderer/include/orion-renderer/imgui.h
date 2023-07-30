#pragma once

#include "orion-core/window.h"

#include "orion-renderapi/render_device.h"

#include <imgui.h>

namespace orion
{
    struct ImGuiInitDesc {
        Window* window;
        RenderDevice* render_device;
        DescriptorPoolHandle descriptor_pool;
        RenderPassHandle render_pass;
    };
} // namespace orion
