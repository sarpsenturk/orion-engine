#include "orion-renderer/mesh.h"

#include "orion-utils/assertion.h"

#include <cstring>

#ifndef ORION_MESH_MANAGER_LOG_LEVEL
    #define ORION_MESH_MANAGER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    Mesh::Mesh(GPUBufferHandle vertex_buffer, GPUBufferHandle index_buffer)
        : vertex_buffer_(vertex_buffer)
        , index_buffer_(index_buffer)
    {
    }
} // namespace orion
