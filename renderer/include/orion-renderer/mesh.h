#pragma once

#include "orion-renderapi/defs.h"
#include "orion-renderapi/device_resource.h"

#include "orion-math/vector/vector3.h"

#include <cstdint>
#include <span>

namespace orion
{
    class Mesh
    {
    public:
        Mesh(UniqueGPUBuffer vertex_buffer, UniqueGPUBuffer index_buffer);

        [[nodiscard]] GPUBufferHandle vertex_buffer() const noexcept { return vertex_buffer_.get(); }
        [[nodiscard]] GPUBufferHandle index_buffer() const noexcept { return index_buffer_.get(); }

    private:
        UniqueGPUBuffer vertex_buffer_;
        UniqueGPUBuffer index_buffer_;
    };

    struct Vertex {
        alignas(16) Vector3_f position;
    };

    using vertex_index_t = std::uint32_t;
    inline constexpr auto vertex_index_type = IndexType::Uint32;

    // Forward declare
    class RenderDevice;
    class CommandAllocator;

    class MeshBuilder
    {
    public:
        MeshBuilder(RenderDevice* device, CommandAllocator* command_allocator);

        Mesh create_mesh(std::span<const Vertex> vertices, std::span<const vertex_index_t> indices);

    private:
        RenderDevice* device_;
        CommandAllocator* command_allocator_;
    };
} // namespace orion
