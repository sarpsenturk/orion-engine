#include "orion-renderer/mesh.h"

#include "orion-renderapi/render_command.h"
#include "orion-renderapi/render_device.h"

#include <cstring>
#include <utility>

#ifndef ORION_MESH_LOG_LEVEL
    #define ORION_MESH_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    namespace
    {
        auto* logger()
        {
            static const auto logger = create_logger("orion-mesh", ORION_MESH_LOG_LEVEL);
            return logger.get();
        }
    } // namespace

    Mesh::Mesh(UniqueGPUBuffer vertex_buffer, UniqueGPUBuffer index_buffer)
        : vertex_buffer_(std::move(vertex_buffer))
        , index_buffer_(std::move(index_buffer))
    {
    }

    MeshBuilder::MeshBuilder(RenderDevice* device, CommandAllocator* command_allocator)
        : device_(device)
        , command_allocator_(command_allocator)
    {
    }

    Mesh MeshBuilder::create_mesh(std::span<const Vertex> vertices, std::span<const vertex_index_t> indices)
    {
        SPDLOG_LOGGER_TRACE(logger(), "Creating mesh...");

        // Create staging buffer
        const auto staging_buffer_size = vertices.size_bytes() + indices.size_bytes();
        auto staging = device_->make_unique<GPUBufferHandle_tag>(GPUBufferDesc{
            .size = staging_buffer_size,
            .usage = GPUBufferUsageFlags::TransferSrc,
            .host_visible = true,
        });
        SPDLOG_LOGGER_TRACE(logger(), "Created staging buffer with size: {} bytes...", staging_buffer_size);

        // Upload vertex & index data to staging buffer
        void* dst = device_->map(staging.get());
        std::memcpy(dst, vertices.data(), vertices.size_bytes());
        std::memcpy(static_cast<char*>(dst) + vertices.size_bytes(), indices.data(), indices.size_bytes());
        device_->unmap(staging.get());
        SPDLOG_LOGGER_TRACE(logger(), "Vertex & index data uploaded to staging buffer...");

        // Create vertex buffer
        auto vertex_buffer = device_->make_unique<GPUBufferHandle_tag>(GPUBufferDesc{
            .size = vertices.size_bytes(),
            .usage = GPUBufferUsageFlags::VertexBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = false,
        });
        SPDLOG_LOGGER_TRACE(logger(), "Created vertex buffer with size: {} bytes", vertices.size_bytes());

        // Create index buffer
        auto index_buffer = device_->make_unique<GPUBufferHandle_tag>(GPUBufferDesc{
            .size = indices.size_bytes(),
            .usage = GPUBufferUsageFlags::IndexBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = false,
        });
        SPDLOG_LOGGER_TRACE(logger(), "Created index buffer with size: {} bytes", indices.size_bytes());

        // Create copy command buffer
        auto command_list = command_allocator_->create_command_list();
        SPDLOG_LOGGER_TRACE(logger(), "Copying vertex & index data to GPU...");

        // Record copy commands
        command_list->begin();
        command_list->copy_buffer({.src = staging.get(), .dst = vertex_buffer.get(), .src_offset = 0, .dst_offset = 0, .size = vertices.size_bytes()});
        command_list->copy_buffer({.src = staging.get(), .dst = index_buffer.get(), .src_offset = vertices.size_bytes(), .dst_offset = 0, .size = indices.size_bytes()});
        command_list->end();

        // Submit and wait for copy
        device_->submit_immediate({.queue_type = CommandQueueType::Graphics, .command_lists = {{command_list.get()}}});
        SPDLOG_LOGGER_TRACE(logger(), "Data copied to GPU.");
        SPDLOG_LOGGER_DEBUG(logger(), "Mesh created.");

        return Mesh{std::move(vertex_buffer), std::move(index_buffer)};
    }
} // namespace orion
