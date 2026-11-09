#pragma once

#include "orion/gpu/gpu.hpp"
#include "orion/gpu/page_allocator.hpp"
#include "orion/gpu/range_allocator.hpp"

#include "orion/utils/slot_map.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace orion
{
    struct Mesh {
        std::uint64_t handle = ~0ull;
    };

    struct MeshRecord {
        gpu::Address position_address;
        gpu::Address uv_address     = 0ull;
        gpu::Address normal_address = 0ull;
        std::uint32_t index_count;
        std::uint32_t first_index;
    };

    static_assert(sizeof(MeshRecord) == 32z);
    static_assert(alignof(MeshRecord) == 8z);
    static_assert(offsetof(MeshRecord, position_address) == 0z);
    static_assert(offsetof(MeshRecord, uv_address) == 8z);
    static_assert(offsetof(MeshRecord, normal_address) == 16z);
    static_assert(offsetof(MeshRecord, index_count) == 24z);
    static_assert(offsetof(MeshRecord, first_index) == 28z);

    struct MeshDesc {
        std::span<const glm::vec3> positions;
        std::span<const glm::vec2> uvs;
        std::span<const glm::vec3> normals;
        std::span<const std::uint32_t> indices;
        std::uint32_t vertex_count;
    };

    class MeshManager
    {
    public:
        // Shared vertex pages and the initial index buffer capacity, in bytes.
        static constexpr std::size_t vertex_page_size          = 16 * 1'024 * 1'024;
        static constexpr std::size_t initial_index_buffer_size = 4 * 1'024 * 1'024;
        // Logical capacity only: uint32 first_index addresses up to 2^32 indices.
        // Actual buffer growth remains subject to device limits and available memory.
        static constexpr std::size_t max_index_buffer_size = (std::size_t{std::numeric_limits<std::uint32_t>::max()} + 1z) * sizeof(std::uint32_t);
        // Maximum number of meshes allowed in MeshManager.
        static constexpr auto max_mesh_count = 65'536u;

        // Device must outlive this manager. GPU work must finish before destruction.
        explicit MeshManager(gpu::Device* device);
        MeshManager(const MeshManager&)            = delete;
        MeshManager& operator=(const MeshManager&) = delete;
        MeshManager(MeshManager&&)                 = delete;
        MeshManager& operator=(MeshManager&&)      = delete;
        ~MeshManager();

        Mesh create_mesh(const MeshDesc& desc);
        void destroy_mesh(Mesh mesh);

        // Includes resources awaiting deferred destruction.
        [[nodiscard]] std::uint32_t mesh_count() const noexcept { return static_cast<std::uint32_t>(m_mesh_records.size()); }

        // Live ranges, including meshes pending destruction. Vertex usage includes
        // padding between attribute sections, but excludes gaps between allocations.
        [[nodiscard]] std::uint64_t vertex_used_bytes() const noexcept;
        [[nodiscard]] std::uint64_t index_used_bytes() const noexcept { return m_index_allocator.used_bytes(); }

        // Actual backing bytes, including unused capacity.
        // The fixed mesh record buffer is reported separately.
        [[nodiscard]] std::uint64_t vertex_allocation_bytes() const noexcept;
        [[nodiscard]] std::uint64_t index_allocation_bytes() const noexcept;
        [[nodiscard]] std::uint64_t record_allocation_bytes() const noexcept { return gpu::buffer_allocation_size(m_mesh_buffer); }

        [[nodiscard]] MeshRecord get_mesh_record(Mesh mesh) const;
        [[nodiscard]] std::uint32_t get_mesh_index(Mesh mesh) const;
        [[nodiscard]] gpu::Address mesh_table_address() const { return gpu::buffer_device_address(m_mesh_buffer); }
        [[nodiscard]] gpu::Buffer* index_buffer() const { return m_index_buffer; }

    private:
        struct VertexPage {
            gpu::Buffer* buffer;
            gpu::RangeAllocator ranges;
        };

        struct VertexAllocation {
            std::size_t page = 0z;
            gpu::RangeAllocator::Allocation range;
        };

        struct MeshEntry {
            MeshRecord record;
            VertexAllocation vertices;
            gpu::RangeAllocator::Allocation indices;
        };

        VertexAllocation allocate_vertices(std::size_t size, std::size_t alignment);
        void grow_index_buffer(std::size_t required_size);

        gpu::Device* m_device;
        gpu::PageAllocator m_vertex_page_allocator;
        std::vector<VertexPage> m_vertex_pages;
        gpu::RangeAllocator m_index_allocator{max_index_buffer_size};

        gpu::Buffer* m_mesh_buffer      = nullptr;
        gpu::Buffer* m_index_buffer     = nullptr;
        std::size_t m_index_buffer_size = initial_index_buffer_size;

        SlotMap<MeshEntry> m_mesh_records;
    };
} // namespace orion
