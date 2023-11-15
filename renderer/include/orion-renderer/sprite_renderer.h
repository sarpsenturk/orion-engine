#pragma once

#include "colors.h"

#include "orion-renderapi/handles.h"

#include "orion-math/vector/vector3.h"

#include <spdlog/logger.h>

namespace orion
{
    struct SpriteComponent {
        Color color;
    };

    // Forward declare
    class RenderDevice;

    struct SpriteRendererDesc {
        RenderDevice* device;
    };

    struct SpriteDrawDesc {
        const SpriteComponent* sprite;
        Vector3_f position;
    };

    class SpriteRenderer
    {
    public:
        explicit SpriteRenderer(const SpriteRendererDesc& desc);

        void draw(const SpriteDrawDesc& draw_desc);
        void reset();

        [[nodiscard]] auto job() const noexcept { return render_job_; }

    private:
        static spdlog::logger* logger();

        RenderDevice* device_;
        GPUJobHandle render_job_;

        struct Stats {
            std::int32_t sprite_count = 0;

            void reset();
        } stats_;
    };
} // namespace orion
