#pragma once

#include "orion-renderer/frame.h"
#include "orion-renderer/render_context.h"
#include "orion-renderer/texture.h"

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
        Vector3_f position = {0.f, 0.f, 0.f};
        Radian_f rotation = radians(0.f);
        Vector2_f scale = {1.f, 1.f};
        Vector4_f color = {1.f, 1.f, 1.f, 1.f};
        texture_index_t texture_index = textures::white;
    };

    // Forward declare
    class RenderDevice;
    class ShaderManager;
    class CommandList;

    struct QuadRendererDesc {
        RenderDevice* device;
        ShaderManager* shader_manager;
        RenderPassHandle render_pass;
        TextureManager* texture_manager;
    };

    class QuadRenderer
    {
    public:
        static constexpr auto quad_vertex_count = 6;

        QuadRenderer(const QuadRendererDesc& desc);

        void begin();
        void add(const QuadData& quad);
        void flush(const RenderContext& render_context);
        //
        //        [[nodiscard]] std::size_t quad_count() const noexcept { return quads_.size(); }
        //        [[nodiscard]] std::size_t vertex_count() const noexcept { return quads_.size() * quad_vertex_count; }

    private:
        struct FrameData {
            MappedGPUBuffer quad_buffer;
            DescriptorHandle buffer_descriptor;
        };

        [[nodiscard]] DescriptorLayoutHandle create_descriptor_layout() const;
        [[nodiscard]] PipelineLayoutHandle create_pipeline_layout() const;
        [[nodiscard]] PipelineHandle create_pipeline(ShaderManager* shader_manager, RenderPassHandle render_pass) const;
        [[nodiscard]] DescriptorPoolHandle create_descriptor_pool() const;
        [[nodiscard]] FrameData create_frame_data() const;

        RenderDevice* device_;
        TextureManager* texture_manager_;
        std::vector<QuadData> quads_;

        UniqueDescriptorLayout descriptor_layout_;
        UniquePipelineLayout pipeline_layout_;
        UniquePipeline pipeline_;
        UniqueDescriptorPool descriptor_pool_;

        PerFrameData<FrameData> frames_;
    };
} // namespace orion
