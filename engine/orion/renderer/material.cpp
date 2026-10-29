#include "material.hpp"

#include "orion/core/assert.hpp"

#include <cstring>
#include <limits>

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
        const auto material_offset = m_material_allocator.allocate(sizeof(GpuPbrMaterial), alignof(GpuPbrMaterial));
        ORION_ASSERT(material_offset != std::numeric_limits<std::size_t>::max());

        std::memcpy(
            static_cast<char*>(gpu::buffer_host_address(m_material_buffer)) + material_offset,
            &material,
            sizeof(GpuPbrMaterial));

        const auto handle = m_material_records.insert({
            .address = gpu::buffer_device_address(m_material_buffer) + material_offset,
        });
        return Material{handle.to_uint64()};
    }

    void MaterialManager::destroy_material(Material material)
    {
        const auto handle = SlotMap<MaterialRecord>::Handle::from_uint64(material.handle);
        m_material_records.erase(handle);
    }

    MaterialRecord MaterialManager::get_material_record(Material material) const
    {
        const auto handle = SlotMap<MaterialRecord>::Handle::from_uint64(material.handle);
        return m_material_records.get(handle);
    }
} // namespace orion
