#include "orion-renderer/sprite_renderer.h"

#include "orion-renderapi/render_device.h"

#ifndef ORION_SPRITE_RENDERER_LOG_LEVEL
    #define ORION_SPRITE_RENDERER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    spdlog::logger* SpriteRenderer::logger()
    {
        static const auto logger = create_logger("orion-sprite", ORION_SPRITE_RENDERER_LOG_LEVEL);
        return logger.get();
    }

    void SpriteRenderer::Stats::reset()
    {
        sprite_count = 0;
    }

    SpriteRenderer::SpriteRenderer(const SpriteRendererDesc& desc)
        : device_(desc.device)
        , render_job_(device_->create_job({.start_finished = true}))
    {
    }

    void SpriteRenderer::draw(const SpriteDrawDesc& draw_desc)
    {
        ++stats_.sprite_count;
    }

    void SpriteRenderer::reset()
    {
        stats_.reset();
    }
} // namespace orion
