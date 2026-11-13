#pragma once

#include "frame.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "orion/utils/slot_map.hpp"

#include <glm/mat4x4.hpp>

#include <cstdint>
#include <span>

namespace orion
{
    struct MeshInstance {
        std::uint64_t handle = ~0ull;
    };

    class MeshRenderer
    {
    public:
        static constexpr std::uint32_t max_instances = 65'536u;

        MeshRenderer(gpu::Device* device, const MeshManager& meshes, const MaterialManager& materials,
                     gpu::Format color_format, gpu::Format depth_format);
        MeshRenderer(const MeshRenderer&) = delete;
        MeshRenderer& operator=(const MeshRenderer&) = delete;
        MeshRenderer(MeshRenderer&&) = delete;
        MeshRenderer& operator=(MeshRenderer&&) = delete;
        ~MeshRenderer();

        MeshInstance create_instance(Mesh mesh, Material material, glm::mat4 transform);
        void remove_instance(MeshInstance instance);
        [[nodiscard]] std::uint32_t instance_count() const noexcept;

        void prepare(Frame& frame, std::span<const ViewIndex> views);

    private:
        struct Instance {
            Mesh mesh;
            Material material;
            glm::mat4 transform;
        };

        gpu::Device* m_device;
        const MeshManager& m_meshes;
        const MaterialManager& m_materials;
        gpu::PipelineState* m_pipeline;
        SlotMap<Instance> m_instances;
    };
} // namespace orion
