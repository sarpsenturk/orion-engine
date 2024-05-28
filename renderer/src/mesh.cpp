#include "orion-renderer/mesh.h"

#include "orion-renderer/render_context.h"

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

    Mesh::Mesh(UniqueGPUBuffer vertex_buffer, UniqueGPUBuffer index_buffer, std::uint32_t index_count)
        : vertex_buffer_(std::move(vertex_buffer))
        , index_buffer_(std::move(index_buffer))
        , index_count_(index_count)
    {
    }

    MeshBuilder::MeshBuilder(RenderContext* render_context)
        : render_context_(render_context)
    {
    }

    Mesh MeshBuilder::create_mesh(std::span<const Vertex> vertices, std::span<const vertex_index_t> indices)
    {
        SPDLOG_LOGGER_TRACE(logger(), "Creating mesh...");

        auto* device = render_context_->device();

        // Create vertex buffer
        auto vertex_buffer = device->create_buffer({
            .size = vertices.size_bytes(),
            .usage = GPUBufferUsageFlags::VertexBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = false,
        });
        SPDLOG_LOGGER_TRACE(logger(), "Created vertex buffer with size: {} bytes", vertices.size_bytes());

        // Create index buffer
        auto index_buffer = device->create_buffer({
            .size = indices.size_bytes(),
            .usage = GPUBufferUsageFlags::IndexBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = false,
        });
        SPDLOG_LOGGER_TRACE(logger(), "Created index buffer with size: {} bytes", indices.size_bytes());

        render_context_->copy_buffer_staging({{
            CopyBufferStaging{
                .bytes = std::as_bytes(vertices),
                .dst = vertex_buffer,
            },
            CopyBufferStaging{
                .bytes = std::as_bytes(indices),
                .dst = index_buffer,
            },
        }});
        SPDLOG_LOGGER_TRACE(logger(), "Data copied to GPU.");
        SPDLOG_LOGGER_DEBUG(logger(), "Mesh created.");
        return Mesh{device->to_unique(vertex_buffer), device->to_unique(index_buffer), static_cast<std::uint32_t>(indices.size())};
    }
} // namespace orion
