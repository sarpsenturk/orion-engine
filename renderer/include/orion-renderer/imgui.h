#pragma once

#include "orion-platform/window.h"

namespace orion
{
    class ImGuiLayer
    {
    public:
        ImGuiLayer() = default;
        virtual ~ImGuiLayer();

        void on_draw();
        void on_event(const WindowEvent& event);

    protected:
        ImGuiLayer(const ImGuiLayer&) = default;
        ImGuiLayer(ImGuiLayer&&) = default;
        ImGuiLayer& operator=(const ImGuiLayer&) = default;
        ImGuiLayer& operator=(ImGuiLayer&&) = default;

    private:
        virtual void on_user_draw();
        virtual void on_user_event(const WindowEvent& event);
    };
} // namespace orion
