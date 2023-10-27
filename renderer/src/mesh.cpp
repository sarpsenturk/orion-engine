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
    Mesh::Mesh(UniqueGPUBuffer vertex_buffer, UniqueGPUBuffer index_buffer, std::uint32_t index_count)
        : vertex_buffer_(std::move(vertex_buffer))
        , index_buffer_(std::move(index_buffer))
        , index_count_(index_count)
    {
    }

    spdlog::logger* MeshManager::logger()
    {
        static const auto logger = create_logger("orion-mesh", ORION_MESH_MANAGER_LOG_LEVEL);
        return logger.get();
    }

    MeshManager::MeshManager(class RenderDevice* device)
        : device_(device)
    {
    }

    std::pair<MeshHandle, const Mesh*> MeshManager::add(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices)
    {
        const auto vb_size = vertices.size_bytes();
        const auto ib_size = indices.size_bytes();

        // Create staging buffer
        auto staging_buffer = device_->create_buffer({
            .size = vertices.size_bytes() + indices.size_bytes(),
            .usage = GPUBufferUsageFlags::TransferSrc,
            .host_visible = true,
        });

        // Copy vertex and index data into staging buffer
        void* ptr = device_->map(staging_buffer);
        std::memcpy(ptr, vertices.data(), vertices.size_bytes());
        ptr = static_cast<char*>(ptr) + vertices.size_bytes();
        std::memcpy(ptr, indices.data(), indices.size_bytes());

        // Create vertex buffer
        auto vertex_buffer = device_->create_buffer({
            .size = vertices.size_bytes(),
            .usage = GPUBufferUsageFlags::VertexBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = false,
        });
        // Create index buffer
        auto index_buffer = device_->create_buffer({
            .size = indices.size_bytes(),
            .usage = GPUBufferUsageFlags::IndexBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = false,
        });

        // TODO: Upload vertex and index data

        // Unmap and destroy staging buffer
        device_->unmap(staging_buffer);
        device_->destroy(staging_buffer);

        // Insert new mesh entry
        const auto handle = next_handle();
        auto [iter, inserted] = meshes_.emplace(handle, Mesh{device_->to_unique(vertex_buffer),
                                                             device_->to_unique(index_buffer),
                                                             static_cast<uint32_t>(indices.size())});
        ORION_ENSURES(inserted);

        auto& mesh = iter->second;
        SPDLOG_LOGGER_DEBUG(logger(), "Created {}: {{ vb: {}, ib: {}, ic: {} }}", iter->first,
                            mesh.vertex_buffer(), mesh.index_buffer(), mesh.index_count());
        return std::make_pair(handle, &mesh);
    }

    void MeshManager::remove(MeshHandle mesh_handle)
    {
        if (const auto erased = meshes_.erase(mesh_handle); erased == 0) {
            SPDLOG_LOGGER_WARN(logger(), "Tying to remove {} which doesn't exist", mesh_handle);
            return;
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Removed {}", mesh_handle);
    }

    void MeshManager::reset()
    {
        meshes_.clear();
        mesh_index_ = 0;
    }

    const Mesh* MeshManager::find(MeshHandle mesh_handle) const noexcept
    {
        if (auto iter = meshes_.find(mesh_handle); iter != meshes_.end()) {
            return &iter->second;
        }
        SPDLOG_LOGGER_WARN(logger(), "Could not find {}", mesh_handle);
        return nullptr;
    }
} // namespace orion
