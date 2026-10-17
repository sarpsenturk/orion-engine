#pragma once

#include "render_graph/resource.hpp"

namespace orion
{
    struct ImGuiRendererDesc {
        const class Window* window = nullptr;
        gpu::Device* device        = nullptr;
        std::uint32_t image_count  = 0u;
        gpu::Format target_format  = gpu::Format::Undefined;
    };

    class ImGuiRenderer
    {
    public:
        explicit ImGuiRenderer(const ImGuiRendererDesc& desc);
        ImGuiRenderer(const ImGuiRenderer&)            = delete;
        ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;
        ImGuiRenderer(ImGuiRenderer&&)                 = delete;
        ImGuiRenderer& operator=(ImGuiRenderer&&)      = delete;
        ~ImGuiRenderer();

        void begin_frame();
        void end_frame();
        void render_platform_windows();

        // The target must match target_format. Load preserves previously rendered content;
        // use Clear when ImGui is the first writer. Execute after end_frame(), before begin_frame().
        [[nodiscard]] RGTexture add_pass(
            class RenderGraph& graph,
            RGTexture target,
            gpu::LoadOp load_op              = gpu::LoadOp::Load,
            gpu::ClearColorValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f});

    private:
        gpu::Device* m_device;
    };
} // namespace orion
