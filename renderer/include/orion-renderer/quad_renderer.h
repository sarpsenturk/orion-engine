#pragma once

#include "orion-renderer/frame.h"
#include "orion-renderer/render_context.h"

#include "orion-renderapi/buffer.h"
#include "orion-renderapi/defs.h"
#include "orion-renderapi/device_resource.h"
#include "orion-renderapi/handles.h"

#include "orion-math/angles.h"
#include "orion-math/matrix/matrix4.h"
#include "orion-math/vector/vector3.h"
#include "orion-math/vector/vector4.h"

#include <vector>

namespace orion
{
    struct QuadData {
        Vector3_f position;
        Radian_f rotation = radians(0.f);
        Vector2_f scale = {1.f, 1.f};
        Vector4_f color;
    };

    struct QuadGPUData {
        Matrix4_f transform;
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
        [[nodiscard]] DescriptorPoolHandle create_descriptor_pool() const;
        [[nodiscard]] FrameData create_frame_data() const;

        RenderDevice* device_;
        std::vector<QuadGPUData> quads_;

        UniqueDescriptorLayout descriptor_layout_;
        UniquePipelineLayout pipeline_layout_;
        UniquePipeline pipeline_;
        UniqueDescriptorPool descriptor_pool_;

        PerFrameData<FrameData> frames_;
    };
} // namespace orion
