#pragma once

#include "gpu/gpu.hpp"

#include "orion/lib/slot_map.hpp"
#include "orion/memory/offset_allocator.hpp"

#include <glm/vec3.hpp>

#include <cstdint>
#include <span>
#include <vector>

namespace orion
{
    struct Geometry {
        gpu::Address positions_base_addr; // Pre-offset base address of the positions buffer
        std::uint32_t vertex_count;       // Number of vertices in the geometry
        std::uint32_t index_count;        // Number of indices in the geometry
        std::uint32_t first_index;        // Offset of the first index in the index buffer
    };

    struct Mesh {
        std::uint64_t handle = ~0ull;
    };

    struct MeshDesc {
        std::span<const glm::vec3> positions;
        std::span<const std::uint32_t> indices;
        std::uint32_t vertex_count;
    };

    class GeometryManager
    {
    public:
        static constexpr std::uint32_t max_mesh_count     = 65'536u;
        static constexpr std::uint32_t attrib_buffer_size = 16u * 1'024u * 1'024u;
        static constexpr std::uint32_t index_buffer_size  = 4u * 1'024u * 1'024u;

        explicit GeometryManager(gpu::Device* device);
        GeometryManager(const GeometryManager&)            = delete;
        GeometryManager& operator=(const GeometryManager&) = delete;
        GeometryManager(GeometryManager&&)                 = delete;
        GeometryManager& operator=(GeometryManager&&)      = delete;
        ~GeometryManager();

        [[nodiscard]] Mesh create_mesh(const MeshDesc& desc);
        void destroy_mesh(Mesh mesh);

        [[nodiscard]] Geometry get_geometry(Mesh mesh) const;
        [[nodiscard]] gpu::Buffer* index_buffer() const { return m_index_buffer; }

    private:
        friend class Renderer;

        struct MeshAllocations {
            OffsetAllocator::Allocation positions;
            OffsetAllocator::Allocation indices;
        };

        struct MeshData {
            Geometry geometry;
            MeshAllocations allocations;
        };

        void retire_meshes(class DeletionQueue& queue);

        gpu::Device* m_device;
        gpu::Buffer* m_positions_buffer;
        gpu::Buffer* m_index_buffer;
        gpu::Address m_positions_address;

        OffsetAllocator m_positions_allocator{attrib_buffer_size, 2u * max_mesh_count + 1u};
        OffsetAllocator m_index_allocator{index_buffer_size, 2u * max_mesh_count + 1u};
        SlotMap<MeshData> m_meshes;
        std::vector<MeshAllocations> m_pending_deletions;
        std::uint32_t m_mesh_count = 0;
    };
} // namespace orion
