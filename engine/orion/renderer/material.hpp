#pragma once

#include "texture.hpp"

#include "orion/gpu/gpu.hpp"

#include "orion/utils/slot_map.hpp"

#include <glm/vec4.hpp>

#include <cstddef>
#include <cstdint>

namespace orion
{
    struct Material {
        std::uint64_t handle = ~0ull;
    };

    struct PbrMaterial {
        glm::vec4 base_color = glm::vec4(1.0f);
        // Optional; the texture must outlive materials that reference it.
        Texture base_color_texture = {};
    };

    class MaterialManager
    {
    public:
        // Maximum number of live materials, including pending destruction.
        static constexpr auto max_material_count = 128u;

        explicit MaterialManager(gpu::Device* device);
        MaterialManager(const MaterialManager&)            = delete;
        MaterialManager& operator=(const MaterialManager&) = delete;
        MaterialManager(MaterialManager&&)                 = delete;
        MaterialManager& operator=(MaterialManager&&)      = delete;
        ~MaterialManager();

        Material create_material(const PbrMaterial& material);
        void destroy_material(Material material);

        // Includes resources awaiting deferred destruction.
        [[nodiscard]] std::uint32_t material_count() const noexcept { return static_cast<std::uint32_t>(m_material_records.size()); }

        // Owned buffer backing allocation in bytes, including unused capacity.
        [[nodiscard]] std::uint64_t allocation_bytes() const noexcept;

        [[nodiscard]] std::uint32_t get_material_index(Material material) const;
        [[nodiscard]] gpu::Address material_table_address() const { return gpu::buffer_device_address(m_material_buffer); }

    private:
        struct alignas(16) GpuPbrMaterial {
            glm::vec4 base_color;
            std::uint32_t base_color_texture;
            std::uint32_t padding[3] = {};
        };
        static_assert(sizeof(GpuPbrMaterial) == 32z);
        static_assert(offsetof(GpuPbrMaterial, base_color) == 0z);
        static_assert(offsetof(GpuPbrMaterial, base_color_texture) == 16z);
        static_assert(offsetof(GpuPbrMaterial, padding) == 20z);
        static constexpr auto material_buffer_size = max_material_count * sizeof(GpuPbrMaterial);

        gpu::Device* m_device;
        gpu::Buffer* m_material_buffer;

        SlotMap<GpuPbrMaterial> m_material_records;
    };
} // namespace orion
