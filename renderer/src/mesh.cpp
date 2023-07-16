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
    Mesh::Mesh(GPUBufferResource vertex_buffer, GPUBufferResource index_buffer)
        : vertex_buffer_(std::move(vertex_buffer))
        , index_buffer_(std::move(index_buffer))
    {
    }
} // namespace orion
