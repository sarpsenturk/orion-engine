#include "mesh.hpp"

#include "orion/core/assert.hpp"

#include <cstring>

namespace orion
{
    MeshManager::MeshManager(gpu::Device* device)
        : m_device(device)
        , m_position_buffer(gpu::create_buffer(
              m_device,
              gpu::BufferDesc{
                  .size   = vertex_buffer_size,
                  .memory = gpu::Memory::CpuVisible,
              }))
        , m_index_buffer(gpu::create_buffer(
              m_device,
              gpu::BufferDesc{
                  .size   = index_buffer_size,
                  .memory = gpu::Memory::CpuVisible,
              }))
    {
    }

    MeshManager::~MeshManager()
    {
        gpu::destroy_buffer(m_device, m_index_buffer);
        gpu::destroy_buffer(m_device, m_position_buffer);
    }

    Mesh MeshManager::create_mesh(const MeshDesc& desc)
    {
        const auto vertex_offset = m_position_allocator.allocate(desc.positions.size_bytes(), alignof(glm::vec3));
        ORION_ASSERT(vertex_offset != std::numeric_limits<std::size_t>::max());
        std::memcpy(
            static_cast<char*>(gpu::buffer_host_address(m_position_buffer)) + vertex_offset,
            desc.positions.data(),
            desc.positions.size_bytes());

        const auto index_offset = m_index_allocator.allocate(desc.indices.size_bytes(), alignof(std::uint32_t));
        ORION_ASSERT(index_offset != std::numeric_limits<std::size_t>::max());
        std::memcpy(
            static_cast<char*>(gpu::buffer_host_address(m_index_buffer)) + index_offset,
            desc.indices.data(),
            desc.indices.size_bytes());

        const auto handle = m_mesh_records.insert({
            .position_address = gpu::buffer_device_address(m_position_buffer) + vertex_offset,
            .index_count      = static_cast<std::uint32_t>(desc.indices.size()),
            .first_index      = static_cast<std::uint32_t>(index_offset / sizeof(std::uint32_t)),
        });
        return Mesh{handle.to_uint64()};
    }

    void MeshManager::destroy_mesh(Mesh mesh)
    {
        const auto handle = SlotMap<MeshRecord>::Handle::from_uint64(mesh.handle);
        m_mesh_records.erase(handle);
    }

    MeshRecord MeshManager::get_mesh_record(Mesh mesh) const
    {
        const auto handle = SlotMap<MeshRecord>::Handle::from_uint64(mesh.handle);
        return m_mesh_records.get(handle);
    }
} // namespace orion
