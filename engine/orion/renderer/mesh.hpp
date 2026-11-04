#pragma once

#include "orion/gpu/bump_allocator.hpp"
#include "orion/gpu/gpu.hpp"

#include "orion/utils/slot_map.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <span>

namespace orion
{
    struct Mesh {
        std::uint64_t handle = ~0ull;
    };

    struct MeshRecord {
        gpu::Address position_address;
        gpu::Address uv_address = 0ull;
        gpu::Address normal_address = 0ull;
        std::uint32_t index_count;
        std::uint32_t first_index;
    };

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
        // Size of vertex attribute buffers in bytes.
        static constexpr auto vertex_buffer_size = 16 * 1'024 * 1'024; // 16 MB
        // Size of index buffers in bytes.
        static constexpr auto index_buffer_size = 4 * 1'024 * 1'024; // 4 MB

        explicit MeshManager(gpu::Device* device);
        MeshManager(const MeshManager&)            = delete;
        MeshManager& operator=(const MeshManager&) = delete;
        MeshManager(MeshManager&&)                 = delete;
        MeshManager& operator=(MeshManager&&)      = delete;
        ~MeshManager();

        Mesh create_mesh(const MeshDesc& desc);
        void destroy_mesh(Mesh mesh);

        [[nodiscard]] MeshRecord get_mesh_record(Mesh mesh) const;
        [[nodiscard]] gpu::Buffer* index_buffer() const { return m_index_buffer; }

    private:
        gpu::Device* m_device;

        gpu::Buffer* m_position_buffer;
        gpu::Buffer* m_index_buffer;
        gpu::Buffer* m_uv_buffer;
        gpu::Buffer* m_normal_buffer;

        gpu::BumpAllocator m_position_allocator{vertex_buffer_size};
        gpu::BumpAllocator m_index_allocator{index_buffer_size};
        gpu::BumpAllocator m_uv_allocator{vertex_buffer_size};
        gpu::BumpAllocator m_normal_allocator{vertex_buffer_size};

        SlotMap<MeshRecord> m_mesh_records;
    };
} // namespace orion
