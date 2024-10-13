#pragma once

#include "orion/renderapi/handle.hpp"

#include "orion/math/matrix/matrix4.hpp"

#include <vector>

namespace orion
{
    // Forward declare
    class RenderDevice;
    class Renderer;

    class SpriteRenderer
    {
    public:
        explicit SpriteRenderer(RenderDevice* render_device);

        void draw();
        void submit(Renderer* renderer);

        [[nodiscard]] std::uint32_t vertex_count() const { return static_cast<std::uint32_t>(sprites_.size()) * 6; }

    private:
        struct SceneData {
            Matrix4f view_projection;
        };

        struct SpriteData {
        };

        DescriptorSetLayoutHandle descriptor_set_layout_;
        PipelineLayoutHandle pipeline_layout_;
        PipelineHandle pipeline_;
        DescriptorPoolHandle descriptor_pool_;
        DescriptorSetHandle descriptor_set_;
        BufferHandle constant_buffer_;

        std::vector<SpriteData> sprites_;
    };
} // namespace orion
