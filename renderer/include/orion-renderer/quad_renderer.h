#pragma once

#include "orion-renderer/frame.h"
#include "orion-renderer/render_context.h"

#include "orion-renderapi/buffer.h"
#include "orion-renderapi/defs.h"
#include "orion-renderapi/handles.h"

#include "orion-math/vector/vector3.h"
#include "orion-math/vector/vector4.h"

#include <vector>

namespace orion
{
    struct QuadData {
        Vector3_f position;
        char pad0;
        Vector4_f color;
    };

    // Forward declare
    class RenderDevice;
    class ShaderManager;
    class CommandList;

    class QuadRenderer
    {
    public:
        static constexpr auto quad_vertex_count = 6;

        QuadRenderer(RenderDevice* device, ShaderManager* shader_manager, RenderPassHandle render_pass);

        void begin();
        void add(const QuadData& quad);
        void flush(const RenderContext& render_context);

        [[nodiscard]] std::size_t quad_count() const noexcept { return quads_.size(); }
        [[nodiscard]] std::size_t vertex_count() const noexcept { return quads_.size() * quad_vertex_count; }

    private:
        struct FrameData {
            MappedGPUBuffer quad_buffer;
            DescriptorHandle descriptor;
        };

        [[nodiscard]] DescriptorLayoutHandle create_descriptor_layout() const;
        [[nodiscard]] PipelineLayoutHandle create_pipeline_layout() const;
        [[nodiscard]] PipelineHandle create_pipeline(ShaderManager* shader_manager, RenderPassHandle render_pass) const;

        RenderDevice* device_;
        std::vector<QuadData> quads_;

        DescriptorLayoutHandle descriptor_layout_;
        PipelineLayoutHandle pipeline_layout_;
        PipelineHandle pipeline_;

        PerFrameData<FrameData> frames_;
    };
} // namespace orion
