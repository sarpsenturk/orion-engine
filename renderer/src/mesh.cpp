#include "orion-renderer/mesh.h"

#include <utility>

namespace orion
{
    Mesh::Mesh(UniqueGPUBuffer vertex_buffer, UniqueGPUBuffer index_buffer, std::uint32_t index_count)
        : vertex_buffer_(std::move(vertex_buffer))
        , index_buffer_(std::move(index_buffer))
        , index_count_(index_count)
    {
    }

} // namespace orion
