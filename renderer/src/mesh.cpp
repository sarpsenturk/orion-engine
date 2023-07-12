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

    MeshManager::MeshManager(RenderDevice* device, CommandPoolHandle command_pool)
        : device_(device)
        , command_buffer_(create_command_buffer(command_pool), std::make_unique<LinearCommandAllocator>(cmd_buffer_size))
    {
    }

    Mesh& MeshManager::add_mesh(const std::string& name, std::span<const Vertex> vertices, std::span<const std::uint32_t> indices)
    {
        ORION_ASSERT(device_ != nullptr);
        ORION_EXPECTS(!name.empty());
        ORION_EXPECTS(!vertices.empty());
        ORION_EXPECTS(!indices.empty());

        if (auto iter = meshes_.find(name); iter != meshes_.end()) {
            return iter->second;
        }

        SPDLOG_LOGGER_DEBUG(logger(), "Creating mesh '{}' with {} vertices and {} indices...", name, vertices.size(), indices.size());
        auto staging_buffer = device_->make_resource(device_->create_buffer({
            .size = vertices.size_bytes() + indices.size_bytes(),
            .usage = GPUBufferUsage::TransferSrc,
            .host_visible = true,
        }));
        SPDLOG_LOGGER_TRACE(logger(), "Staging buffer with size {} created...", vertices.size_bytes() + indices.size_bytes());

        void* mapped_buffer = device_->map(staging_buffer.get());
        std::memcpy(mapped_buffer, vertices.data(), vertices.size_bytes());
        mapped_buffer = static_cast<char*>(mapped_buffer) + vertices.size_bytes();
        std::memcpy(mapped_buffer, indices.data(), indices.size_bytes());
        SPDLOG_LOGGER_TRACE(logger(), "Vertex and index data memcpy'd to staging buffer...");

        auto vb_handle = device_->create_buffer({
            .size = vertices.size_bytes(),
            .usage = GPUBufferUsageFlags::disjunction({GPUBufferUsage::VertexBuffer, GPUBufferUsage::TransferDst}),
            .host_visible = false,
        });
        SPDLOG_LOGGER_TRACE(logger(), "Vertex buffer created...");
        auto ib_handle = device_->create_buffer({
            .size = indices.size_bytes(),
            .usage = GPUBufferUsage::IndexBuffer,
            .host_visible = false,
        });
        SPDLOG_LOGGER_TRACE(logger(), "Index buffer created...");

        // Copy command vertex buffer
        {
            auto* vb_copy = command_buffer_.add_command<CmdBufferCopy>({});
            vb_copy->dst = vb_handle;
            vb_copy->dst_offset = 0;
            vb_copy->src = staging_buffer.get();
            vb_copy->src_offset = 0;
            vb_copy->size = vertices.size_bytes();
        }

        // Copy command index buffer
        {
            auto* ib_copy = command_buffer_.add_command<CmdBufferCopy>({});
            ib_copy->dst = ib_handle;
            ib_copy->dst_offset = vertices.size_bytes();
            ib_copy->src = staging_buffer.get();
            ib_copy->src_offset = vertices.size_bytes();
            ib_copy->size = indices.size_bytes();
        }

        device_->submit_immediate({.command_buffer = &command_buffer_, .queue_type = CommandQueueType::Transfer});
        command_buffer_.reset();
        SPDLOG_LOGGER_TRACE(logger(), "Vertex and index data copied to gpu buffers");
        SPDLOG_LOGGER_DEBUG(logger(), "Mesh '{}' created.", name);

        auto [iter, success] = meshes_.insert(std::make_pair(name, Mesh({vb_handle, device_}, {ib_handle, device_})));
        ORION_EXPECTS(success);
        return iter->second;
    }

    spdlog::logger* MeshManager::logger()
    {
        static auto logger = create_logger("orion-mesh-manager", static_cast<spdlog::level::level_enum>(ORION_MESH_MANAGER_LOG_LEVEL));
        return logger.get();
    }

    CommandBufferHandle MeshManager::create_command_buffer(CommandPoolHandle command_pool) const
    {
        ORION_ASSERT(device_ != nullptr);
        return device_->create_command_buffer({.command_pool = command_pool});
    }

    const Mesh& MeshManager::get_mesh(const std::string& name) const
    {
        return meshes_.at(name);
    }
} // namespace orion
