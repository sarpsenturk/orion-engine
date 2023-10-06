#include "orion-renderer/shader.h"

#include "orion-utils/bitflag.h"

#ifndef ORION_SHADER_MANAGER_LOG_LEVEL
    #define ORION_SHADER_MANAGER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    ShaderManager::ShaderManager(RenderDevice* device)
        : device_(device)
    {
    }
} // namespace orion
