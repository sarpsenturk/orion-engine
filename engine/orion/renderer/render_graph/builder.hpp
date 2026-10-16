#pragma once

#include "resource.hpp"

#include "orion/renderer/gpu/gpu.hpp"

#include <cstdint>

namespace orion
{
    class RenderPassBuilder
    {
    public:
        RGTexture add_color_attachment(
            RGTexture texture,
            gpu::LoadOp load_op              = gpu::LoadOp::Clear,
            gpu::StoreOp store_op            = gpu::StoreOp::Store,
            gpu::ClearColorValue clear_color = {0.0f, 0.0f, 0.0f, 1.0f});

    private:
        friend class RenderGraph;
        RenderPassBuilder(class RenderGraph* graph, std::uint32_t pass_index);

        class RenderGraph* m_graph;
        std::uint32_t m_pass_index;
    };
} // namespace orion
