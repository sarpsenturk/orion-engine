#include "material.hpp"

#include "orion/core/assert.hpp"

#include <cstring>
#include <stdexcept>

namespace orion
{
    MaterialManager::MaterialManager(gpu::Device* device)
        : m_device(device)
        , m_material_buffer(gpu::create_buffer(
              device,
              gpu::BufferDesc{material_buffer_size, gpu::Memory::CpuVisible}))
    {
    }

    MaterialManager::~MaterialManager()
    {
        gpu::destroy_buffer(m_device, m_material_buffer);
    }

    Material MaterialManager::create_material(const PbrMaterial& material)
    {
        if (m_material_records.size() >= max_material_count) {
            throw std::length_error("Material table capacity exhausted");
        }

        // Texture descriptors use the stable slot index, not the generation
        // encoded in the upper half of the CPU handle. The empty handle maps
        // to ~0u, which denotes an untextured material in the shader.
        const auto gpu_material = GpuPbrMaterial{
            .base_color = material.base_color,
            .base_color_texture = static_cast<std::uint32_t>(material.base_color_texture.handle),
        };
        const auto handle = m_material_records.insert(gpu_material);
        const auto material_offset = handle.index * sizeof(GpuPbrMaterial);
        std::memcpy(
            static_cast<char*>(gpu::buffer_host_address(m_material_buffer)) + material_offset,
            &gpu_material,
            sizeof(GpuPbrMaterial));

        return Material{handle.to_uint64()};
    }

    void MaterialManager::destroy_material(Material material)
    {
        const auto handle = SlotMap<GpuPbrMaterial>::Handle::from_uint64(material.handle);
        m_material_records.erase(handle);
    }

    std::uint32_t MaterialManager::get_material_index(Material material) const
    {
        const auto handle = SlotMap<GpuPbrMaterial>::Handle::from_uint64(material.handle);
        ORION_ASSERT(m_material_records.contains(handle));
        return handle.index;
    }
} // namespace orion
