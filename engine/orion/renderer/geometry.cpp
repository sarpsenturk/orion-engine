#include "geometry.hpp"

#include "gpu/deletion_queue.hpp"
#include "orion/core/assert.hpp"

#include <utility>

namespace orion
{
    GeometryManager::GeometryManager(gpu::Device* device)
        : m_device(device)
        , m_positions_buffer(gpu::create_buffer(
              device,
              gpu::BufferDesc{
                  .size   = attrib_buffer_size,
                  .memory = gpu::Memory::Default,
                  .usage  = gpu::BufferUsage::DeviceAddress,
              }))
        , m_index_buffer(gpu::create_buffer(
              device,
              gpu::BufferDesc{
                  .size   = index_buffer_size,
                  .memory = gpu::Memory::Default,
                  .usage  = gpu::BufferUsage::IndexBuffer,
              }))
        , m_positions_address(gpu::get_buffer_device_address(m_positions_buffer))
    {
    }

    GeometryManager::~GeometryManager()
    {
        gpu::wait_idle(m_device);
        gpu::destroy_buffer(m_device, m_index_buffer);
        gpu::destroy_buffer(m_device, m_positions_buffer);
    }

    Mesh GeometryManager::create_mesh(const MeshDesc& desc)
    {
        ORION_ASSERT(m_mesh_count < max_mesh_count);
        ORION_ASSERT(!desc.positions.empty());
        ORION_ASSERT(desc.positions.size() == desc.vertex_count);
        ORION_ASSERT(desc.positions.size_bytes() <= attrib_buffer_size);
        ORION_ASSERT(desc.indices.size_bytes() <= index_buffer_size);

        const auto positions = m_positions_allocator.allocate(
            static_cast<std::uint32_t>(desc.positions.size_bytes()), alignof(glm::vec3));
        ORION_ASSERT(positions);

        auto indices = OffsetAllocator::Allocation{};
        if (!desc.indices.empty()) {
            indices = m_index_allocator.allocate(
                static_cast<std::uint32_t>(desc.indices.size_bytes()), alignof(std::uint32_t));
            ORION_ASSERT(indices);
            gpu::copy_to_buffer(m_device, m_index_buffer, indices.offset, std::as_bytes(desc.indices));
        }
        gpu::copy_to_buffer(m_device, m_positions_buffer, positions.offset, std::as_bytes(desc.positions));

        const auto handle = m_meshes.emplace(MeshData{
            .geometry = {
                .positions_base_addr = m_positions_address + positions.offset,
                .vertex_count        = desc.vertex_count,
                .index_count         = static_cast<std::uint32_t>(desc.indices.size()),
                .first_index         = indices ? indices.offset / static_cast<std::uint32_t>(sizeof(std::uint32_t)) : 0u,
            },
            .allocations = {
                .positions = positions,
                .indices   = indices,
            },
        });
        ++m_mesh_count;
        return Mesh{.handle = handle.to_uint64()};
    }

    void GeometryManager::destroy_mesh(Mesh mesh)
    {
        const auto handle = SlotMap<MeshData>::Handle::from_uint64(mesh.handle);
        const auto* data  = m_meshes.get(handle);
        ORION_ASSERT(data != nullptr);
        m_pending_deletions.push_back(data->allocations);
        m_meshes.erase(handle);
    }

    Geometry GeometryManager::get_geometry(Mesh mesh) const
    {
        const auto* data = m_meshes.get(SlotMap<MeshData>::Handle::from_uint64(mesh.handle));
        ORION_ASSERT(data != nullptr);
        return data->geometry;
    }

    void GeometryManager::retire_meshes(DeletionQueue& queue)
    {
        if (m_pending_deletions.empty()) return;

        queue.push([this, allocations = std::exchange(m_pending_deletions, {})](gpu::Device*) {
            for (const auto& allocation : allocations) {
                m_positions_allocator.deallocate(allocation.positions);
                if (allocation.indices) m_index_allocator.deallocate(allocation.indices);
                --m_mesh_count;
            }
        });
    }
} // namespace orion
