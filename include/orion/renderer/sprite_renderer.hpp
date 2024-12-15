#pragma once

#include "orion/renderer/camera.hpp"

#include "orion/renderapi/handle.hpp"

#include "orion/math/matrix/matrix4.hpp"
#include "orion/math/vector/vector2.hpp"
#include "orion/math/vector/vector3.hpp"

#include <vector>

namespace orion
{
    // Forward declare
    class RenderDevice;
    class Renderer;

    struct SpriteRendererDesc {
        RenderDevice* render_device;
        std::uint32_t screen_width;
        std::uint32_t screen_height;
        float camera_size;
    };

    class SpriteRenderer
    {
    public:
        static constexpr auto max_batch_size = 5000;

        explicit SpriteRenderer(const SpriteRendererDesc& desc);

        void draw(const Vector3f& position, const Vector2f& size);
        void submit(Renderer* renderer);

        [[nodiscard]] std::uint32_t vertex_count() const { return static_cast<std::uint32_t>(sprites_.size()) * 6; }

    private:
        struct SceneData {
            Matrix4f view_projection;
        };

        struct SpriteData {
            Matrix4f model;
        };

        static constexpr auto sprite_buffer_size = sizeof(SpriteData) * max_batch_size;

        DescriptorLayoutHandle descriptor_set_layout_;
        PipelineLayoutHandle pipeline_layout_;
        PipelineHandle pipeline_;
        DescriptorHandle descriptor_set_;
        BufferHandle constant_buffer_;
        BufferHandle sprite_buffer_;

        std::vector<SpriteData> sprites_;
        Camera camera_;
    };
} // namespace orion
