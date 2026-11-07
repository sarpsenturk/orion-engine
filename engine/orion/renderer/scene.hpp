#pragma once

#include "material.hpp"
#include "mesh.hpp"

#include "orion/gpu/gpu.hpp"
#include "orion/utils/slot_map.hpp"

#include <glm/mat4x4.hpp>

#include <cstdint>

namespace orion
{
    struct RenderInstance {
        std::uint64_t handle = ~0ull;
    };

    struct RenderInstanceRecord {
        Mesh mesh;
        Material material;
        glm::mat4 transform;
    };

    class Scene
    {
    public:
        // Maximum number of live instances in a Scene.
        static constexpr auto max_instances = 65'536;

        explicit Scene(gpu::Device* device);
        Scene(const Scene&)            = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&)                 = delete;
        Scene& operator=(Scene&&)      = delete;
        ~Scene();

        RenderInstance create_instance(Mesh mesh, Material material, glm::mat4 transform);
        void remove_instance(RenderInstance instance);

        [[nodiscard]] gpu::Address scene_buffer_address() const { return m_scene_buffer_addr; }
        [[nodiscard]] auto& instances() const { return m_instance_slots; }

    private:
        struct alignas(16) GpuInstance {
            std::uint32_t mesh_idx;
            std::uint32_t material_idx;
            std::uint32_t padding[2];
            glm::mat4 transform;
        };

        struct alignas(16) GpuSceneHeader {
            gpu::Address instances;
            char padding[8];
        };

        static constexpr auto instance_buffer_size = sizeof(GpuInstance) * max_instances;
        static constexpr auto scene_buffer_size    = sizeof(GpuSceneHeader) + instance_buffer_size;

        gpu::Device* m_device;
        gpu::Buffer* m_scene_buffer;
        gpu::Address m_scene_buffer_addr;
        gpu::Address m_instance_addr;
        SlotMap<RenderInstanceRecord> m_instance_slots;
    };
} // namespace orion
