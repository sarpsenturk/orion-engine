#pragma once

#include "vertex.h"

#include "orion-core/handle.h"

#include "orion-renderapi/defs.h"
#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_device.h"

#include "orion-math/vector/vector3.h"
#include "orion-math/vector/vector4.h"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <unordered_map>
#include <utility>

#include <spdlog/logger.h>

namespace orion
{
    class Mesh
    {
    public:
        Mesh(UniqueGPUBuffer vertex_buffer, UniqueGPUBuffer index_buffer, std::uint32_t index_count);

        [[nodiscard]] auto vertex_buffer() const noexcept { return vertex_buffer_.get(); }
        [[nodiscard]] auto index_buffer() const noexcept { return index_buffer_.get(); }
        [[nodiscard]] auto index_count() const noexcept { return index_count_; }

    private:
        UniqueGPUBuffer vertex_buffer_;
        UniqueGPUBuffer index_buffer_;
        std::uint32_t index_count_;
    };

    using mesh_handle_key_t = std::uint16_t;
    ORION_DEFINE_HANDLE(MeshHandle, mesh_handle_key_t);

    class MeshManager
    {
    public:
        explicit MeshManager(RenderDevice* device);

        std::pair<MeshHandle, const Mesh*> add(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices);
        void remove(MeshHandle mesh_handle);
        void reset();

        [[nodiscard]] const Mesh* find(MeshHandle mesh_handle) const noexcept;

    private:
        MeshHandle next_handle() noexcept { return MeshHandle{mesh_index_++}; }

        RenderDevice* device_;
        std::unordered_map<MeshHandle, Mesh> meshes_;
        mesh_handle_key_t mesh_index_ = 0;

        static spdlog::logger* logger();
    };

    namespace default_meshes
    {
        inline constexpr auto cube_vertices = std::array{
            // Front vertices
            Vertex{.position = {-.5f, -.5f, 0.5f}, .color = {1.f, 1.f, 1.f, 1.f}},
            Vertex{.position = {0.5f, -.5f, 0.5f}, .color = {1.f, 1.f, 1.f, 1.f}},
            Vertex{.position = {0.5f, 0.5f, 0.5f}, .color = {1.f, 1.f, 1.f, 1.f}},
            Vertex{.position = {-.5f, 0.5f, 0.5f}, .color = {1.f, 1.f, 1.f, 1.f}},
            // Back vertices
            Vertex{.position = {-.5f, -.5f, -0.5f}, .color = {1.f, 1.f, 1.f, 1.f}},
            Vertex{.position = {0.5f, -.5f, -0.5f}, .color = {1.f, 1.f, 1.f, 1.f}},
            Vertex{.position = {0.5f, 0.5f, -0.5f}, .color = {1.f, 1.f, 1.f, 1.f}},
            Vertex{.position = {-.5f, 0.5f, -0.5f}, .color = {1.f, 1.f, 1.f, 1.f}},
        };
        inline constexpr auto cube_indices = std::array{
            // Front face
            0u, 1u, 2u,
            2u, 3u, 0u,
            // Right face
            1u, 5u, 6u,
            6u, 2u, 1u,
            // Back face
            5u, 4u, 6u,
            7u, 6u, 5u,
            // Left face
            4u, 0u, 3u,
            3u, 7u, 4u,
            // Bottom face
            3u, 2u, 6u,
            6u, 7u, 3u,
            // Top face
            4u, 5u, 1u,
            1u, 0u, 4u};
    } // namespace default_meshes
} // namespace orion
