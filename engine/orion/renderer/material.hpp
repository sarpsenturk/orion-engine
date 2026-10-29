#pragma once

#include "orion/gpu/bump_allocator.hpp"
#include "orion/gpu/gpu.hpp"

#include "orion/utils/slot_map.hpp"

#include <glm/vec4.hpp>

#include <cstdint>

namespace orion
{
    struct Material {
        std::uint64_t handle = ~0ull;
    };

    struct MaterialRecord {
        gpu::Address address = 0ull;
    };

    struct PbrMaterial {
        glm::vec4 base_color = glm::vec4(1.0f);
    };

    class MaterialManager
    {
    public:
        // Maximum number of materials that can be created.
        static constexpr auto max_material_count = 128u;

        explicit MaterialManager(gpu::Device* device);
        MaterialManager(const MaterialManager&)            = delete;
        MaterialManager& operator=(const MaterialManager&) = delete;
        MaterialManager(MaterialManager&&)                 = delete;
        MaterialManager& operator=(MaterialManager&&)      = delete;
        ~MaterialManager();

        Material create_material(const PbrMaterial& material);
        void destroy_material(Material material);

        [[nodiscard]] MaterialRecord get_material_record(Material material) const;

    private:
        struct GpuPbrMaterial {
            glm::vec4 base_color;
        };
        static constexpr auto material_buffer_size = max_material_count * sizeof(GpuPbrMaterial);

        gpu::Device* m_device;
        gpu::Buffer* m_material_buffer;

        gpu::BumpAllocator m_material_allocator{material_buffer_size};

        SlotMap<MaterialRecord> m_material_records;
    };
} // namespace orion
