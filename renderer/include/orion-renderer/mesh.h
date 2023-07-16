#pragma once

#include <orion-renderapi/command.h>
#include <orion-renderapi/handles.h>
#include <orion-renderapi/pipeline.h>
#include <orion-renderapi/render_device.h>

#include <orion-math/vector/vector3.h>
#include <orion-math/vector/vector4.h>

#include <array>
#include <span>
#include <spdlog/logger.h>
#include <string>
#include <unordered_map>

namespace orion
{
    struct Vertex {
        Vector3_f position;
        Vector4_f color;

        static constexpr auto vertex_bindings()
        {
            std::array attributes{
                VertexAttributeDesc{.name = "POSITION", .format = Format::R32G32B32_Float},
                VertexAttributeDesc{.name = "COLOR", .format = Format::R32G32B32A32_Float},
            };
            return std::array{
                VertexBinding(attributes, InputRate::Vertex),
            };
        }
    };

    inline constexpr auto quad_vertex_positions = std::array{
        Vector3_f{-.5f, 0.5f, 0.0f},
        Vector3_f{0.5f, 0.5f, 0.0f},
        Vector3_f{0.5f, -.5f, 0.0f},
        Vector3_f{-.5f, -.5f, 0.0f},
    };
    inline constexpr auto quad_indices = std::array{
        0u, 1u, 2u,
        2u, 3u, 0u};

    class Mesh
    {
    public:
        Mesh(GPUBufferResource vertex_buffer, GPUBufferResource index_buffer);

        [[nodiscard]] auto vertex_buffer() const noexcept { return vertex_buffer_.get(); }
        [[nodiscard]] auto index_buffer() const noexcept { return index_buffer_.get(); }

    private:
        GPUBufferResource vertex_buffer_;
        GPUBufferResource index_buffer_;
    };
} // namespace orion
