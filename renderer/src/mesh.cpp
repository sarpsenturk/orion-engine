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

    const Mesh* MeshManager::add(std::string name, std::span<const Vertex> vertices, std::span<const std::uint32_t> indices)
    {
        if (auto iter = meshes_.find(name); iter != meshes_.end()) {
            SPDLOG_LOGGER_WARN(logger(), "Mesh with name {} already exists, returning existing mesh.");
            return &iter->second;
        }

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

        // Upload vertex and index data
        device_->submit_immediate([vertex_buffer, index_buffer, staging_buffer, vb_size, ib_size]() {
            CommandList upload{sizeof(CmdBufferCopy) * 2};
            upload.begin();
            // Copy vertex data
            {
                auto* cmd_buffer_copy = upload.add_command<CmdBufferCopy>({});
                cmd_buffer_copy->dst = vertex_buffer;
                cmd_buffer_copy->dst_offset = 0;
                cmd_buffer_copy->src = staging_buffer;
                cmd_buffer_copy->src_offset = 0;
                cmd_buffer_copy->size = vb_size;
            }
            // Copy index data
            {
                auto* cmd_buffer_copy = upload.add_command<CmdBufferCopy>({});
                cmd_buffer_copy->dst = index_buffer;
                cmd_buffer_copy->dst_offset = 0;
                cmd_buffer_copy->src = staging_buffer;
                cmd_buffer_copy->src_offset = vb_size;
                cmd_buffer_copy->size = ib_size;
            }
            upload.end();
            return upload;
        });

        // Unmap and destroy staging buffer
        device_->unmap(staging_buffer);
        device_->destroy(staging_buffer);

        // Insert new mesh entry
        auto [iter, inserted] = meshes_.emplace(std::move(name), Mesh{device_->to_unique(vertex_buffer),
                                                                      device_->to_unique(index_buffer),
                                                                      static_cast<uint32_t>(indices.size())});
        ORION_ENSURES(inserted);

        auto& mesh = iter->second;
        SPDLOG_LOGGER_DEBUG(logger(), "Mesh '{}' created: {{ vb: {}, ib: {}, ic: {} }}", iter->first,
                            mesh.vertex_buffer(), mesh.index_buffer(), mesh.index_count());
        return &mesh;
    }

    void MeshManager::remove(const std::string& name)
    {
        if (const auto erased = meshes_.erase(name); erased == 0) {
            SPDLOG_LOGGER_WARN(logger(), "Tying to remove mesh '{}' which doesn't exist", name);
            return;
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Removed mesh '{}'", name);
    }

    const Mesh* MeshManager::find(const std::string& name) const
    {
        if (auto iter = meshes_.find(name); iter != meshes_.end()) {
            return &iter->second;
        }
        SPDLOG_LOGGER_WARN(logger(), "Could not find mesh '{}'", name);
        return nullptr;
    }
} // namespace orion
