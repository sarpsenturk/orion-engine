#include "mesh.hpp"

#include "orion/core/assert.hpp"
#include "orion/utils/scope_guard.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>

namespace orion
{
    MeshManager::MeshManager(gpu::Device* device)
        : m_device(device)
        , m_vertex_page_allocator(device, {.size = vertex_page_size, .memory = gpu::Memory::CpuVisible})
        , m_mesh_buffer(gpu::create_buffer(
              m_device,
              gpu::BufferDesc{
                  .size   = max_mesh_count * sizeof(MeshRecord),
                  .memory = gpu::Memory::CpuVisible,
              }))
        , m_index_buffer(gpu::create_buffer(
              m_device,
              gpu::BufferDesc{
                  .size   = initial_index_buffer_size,
                  .memory = gpu::Memory::CpuVisible,
              }))
    {
    }

    MeshManager::~MeshManager()
    {
        gpu::destroy_buffer(m_device, m_index_buffer);
        gpu::destroy_buffer(m_device, m_mesh_buffer);
    }

    MeshManager::VertexAllocation MeshManager::allocate_vertices(std::size_t size, std::size_t alignment)
    {
        for (std::size_t i = 0z; i < m_vertex_pages.size(); ++i) {
            const auto range = m_vertex_pages[i].ranges.allocate(size, alignment);
            if (range) { return {i, range}; }
        }

        const auto page_size = std::max(vertex_page_size, size);
        auto ranges          = gpu::RangeAllocator{page_size};
        const auto range     = ranges.allocate(size, alignment);
        if (!range) { throw std::bad_alloc{}; }
        // Finish CPU allocations before acquiring the page so no retained page
        // can be left outside m_vertex_pages if allocation fails.
        if (m_vertex_pages.size() == m_vertex_pages.capacity()) {
            m_vertex_pages.reserve(std::max(std::size_t{1}, m_vertex_pages.size() * 2z));
        }
        auto* buffer = m_vertex_page_allocator.acquire(page_size);
        m_vertex_pages.push_back({buffer, std::move(ranges)});
        return {m_vertex_pages.size() - 1z, range};
    }

    void MeshManager::grow_index_buffer(std::size_t required_size)
    {
        if (required_size <= m_index_buffer_size) { return; }
        if (required_size > max_index_buffer_size) {
            throw std::length_error("Mesh index address space exhausted");
        }
        const auto size  = std::max(required_size, std::min(max_index_buffer_size, m_index_buffer_size * 2z));
        auto* buffer     = gpu::create_buffer(m_device, {.size = size, .memory = gpu::Memory::CpuVisible});
        const auto guard = ScopeFail{[&] { gpu::destroy_buffer(m_device, buffer); }};
        // Growth is infrequent. Finish submitted GPU reads before replacing the
        // buffer, then preserve every first_index by copying at unchanged offsets.
        gpu::wait_idle(m_device);
        std::memcpy(gpu::buffer_host_address(buffer), gpu::buffer_host_address(m_index_buffer), m_index_buffer_size);
        gpu::destroy_buffer(m_device, m_index_buffer);
        m_index_buffer      = buffer;
        m_index_buffer_size = size;
    }

    std::uint64_t MeshManager::vertex_used_bytes() const noexcept
    {
        auto bytes = std::uint64_t{0};
        for (const auto& page : m_vertex_pages) { bytes += page.ranges.used_bytes(); }
        return bytes;
    }

    std::uint64_t MeshManager::vertex_allocation_bytes() const noexcept
    {
        auto bytes = std::uint64_t{0};
        for (const auto& page : m_vertex_pages) { bytes += gpu::buffer_allocation_size(page.buffer); }
        return bytes;
    }

    std::uint64_t MeshManager::index_allocation_bytes() const noexcept
    {
        return gpu::buffer_allocation_size(m_index_buffer);
    }

    Mesh MeshManager::create_mesh(const MeshDesc& desc)
    {
        // Slot indices cannot exceed the peak live count when slots are reused.
        if (m_mesh_records.size() >= max_mesh_count) {
            throw std::length_error("Mesh table capacity exhausted");
        }
        ORION_ASSERT(desc.uvs.empty() || desc.uvs.size() == desc.positions.size());
        ORION_ASSERT(desc.normals.empty() || desc.normals.size() == desc.positions.size());
        if (desc.indices.size() > std::numeric_limits<std::uint32_t>::max()) {
            throw std::length_error("Mesh index count exceeds draw limit");
        }

        // One allocation per mesh, with contiguous SoA sections. Align each
        // section independently, including when aligned GLM types are enabled.
        auto vertex_size  = std::size_t{0};
        const auto append = [&](std::size_t size, std::size_t alignment) {
            const auto padding = (alignment - vertex_size % alignment) % alignment;
            if (vertex_size > std::numeric_limits<std::size_t>::max() - padding ||
                size > std::numeric_limits<std::size_t>::max() - vertex_size - padding) {
                throw std::length_error("Mesh vertex data is too large");
            }
            const auto offset = vertex_size + padding;
            vertex_size       = offset + size;
            return offset;
        };
        const auto position_offset = append(desc.positions.size_bytes(), alignof(glm::vec3));
        const auto uv_offset       = desc.uvs.empty() ? 0z : append(desc.uvs.size_bytes(), alignof(glm::vec2));
        const auto normal_offset   = desc.normals.empty() ? 0z : append(desc.normals.size_bytes(), alignof(glm::vec3));
        const auto vertices        = vertex_size == 0z ? VertexAllocation{} : allocate_vertices(vertex_size, alignof(glm::vec3));
        const auto vertex_guard    = ScopeFail{[&] {
            if (vertices.range) { m_vertex_pages[vertices.page].ranges.free(vertices.range); }
        }};

        auto indices           = gpu::RangeAllocator::Allocation{};
        const auto index_guard = ScopeFail{[&] {
            if (indices) { m_index_allocator.free(indices); }
        }};
        if (!desc.indices.empty()) {
            indices = m_index_allocator.allocate(desc.indices.size_bytes(), alignof(std::uint32_t));
            if (!indices) { throw std::length_error("Mesh index address space exhausted"); }
            grow_index_buffer(indices.offset + indices.size);
            std::memcpy(static_cast<char*>(gpu::buffer_host_address(m_index_buffer)) + indices.offset,
                        desc.indices.data(), desc.indices.size_bytes());
        }

        auto record = MeshRecord{
            .position_address = 0ull,
            .index_count      = static_cast<std::uint32_t>(desc.indices.size()),
            .first_index      = static_cast<std::uint32_t>(indices.offset / sizeof(std::uint32_t)),
        };
        if (vertices.range) {
            auto* buffer       = m_vertex_pages[vertices.page].buffer;
            auto* host         = static_cast<char*>(gpu::buffer_host_address(buffer)) + vertices.range.offset;
            const auto address = gpu::buffer_device_address(buffer) + vertices.range.offset;
            if (!desc.positions.empty()) {
                std::memcpy(host + position_offset, desc.positions.data(), desc.positions.size_bytes());
                record.position_address = address + position_offset;
            }
            if (!desc.uvs.empty()) {
                std::memcpy(host + uv_offset, desc.uvs.data(), desc.uvs.size_bytes());
                record.uv_address = address + uv_offset;
            }
            if (!desc.normals.empty()) {
                std::memcpy(host + normal_offset, desc.normals.data(), desc.normals.size_bytes());
                record.normal_address = address + normal_offset;
            }
        }

        const auto handle = m_mesh_records.insert({record, vertices, indices});
        // GPU records use stable handle slots, not the SlotMap's dense positions.
        std::memcpy(
            static_cast<char*>(gpu::buffer_host_address(m_mesh_buffer)) + handle.index * sizeof(MeshRecord),
            &record,
            sizeof(record));
        return Mesh{handle.to_uint64()};
    }

    void MeshManager::destroy_mesh(Mesh mesh)
    {
        const auto handle = SlotMap<MeshEntry>::Handle::from_uint64(mesh.handle);
        const auto* entry = m_mesh_records.try_get(handle);
        if (!entry) { return; }
        // Renderer defers this call until all GPU readers have completed.
        if (entry->vertices.range) {
            m_vertex_pages[entry->vertices.page].ranges.free(entry->vertices.range);
        }
        if (entry->indices) { m_index_allocator.free(entry->indices); }
        m_mesh_records.erase(handle);
    }

    std::uint32_t MeshManager::get_mesh_index(Mesh mesh) const
    {
        const auto handle = SlotMap<MeshEntry>::Handle::from_uint64(mesh.handle);
        ORION_ASSERT(m_mesh_records.contains(handle));
        return handle.index;
    }

    MeshRecord MeshManager::get_mesh_record(Mesh mesh) const
    {
        const auto handle = SlotMap<MeshEntry>::Handle::from_uint64(mesh.handle);
        return m_mesh_records.get(handle).record;
    }
} // namespace orion
