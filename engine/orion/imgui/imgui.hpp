#pragma once

#include "orion/gpu/gpu.hpp"

#include <imgui.h>

namespace orion
{
    class ImGuiContext
    {
    public:
        ImGuiContext(
            const class Renderer& renderer,
            const class Window& window);
        ImGuiContext(const ImGuiContext&)            = delete;
        ImGuiContext& operator=(const ImGuiContext&) = delete;
        ImGuiContext(ImGuiContext&&)                 = delete;
        ImGuiContext& operator=(ImGuiContext&&)      = delete;
        ~ImGuiContext();

        void begin_frame();
        void end_frame();
        void update_platform_windows();

        [[nodiscard]] bool wants_capture_mouse() const;
        [[nodiscard]] bool wants_capture_keyboard() const;
    };
} // namespace orion
