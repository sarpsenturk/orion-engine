#include "scene.hpp"

#include <cstring>
#include <stdexcept>

namespace orion
{
    Scene::Scene(gpu::Device* device)
        : m_device(device)
        , m_scene_buffer(gpu::create_buffer(
              m_device,
              gpu::BufferDesc{
                  .size   = scene_buffer_size,
                  .memory = gpu::Memory::CpuVisible,
              }))
        , m_scene_buffer_addr(gpu::buffer_device_address(m_scene_buffer))
        , m_instance_addr(m_scene_buffer_addr + sizeof(GpuSceneHeader))
    {
        // Write the scene header into the buffer
        const auto scene_header = GpuSceneHeader{.instances = m_instance_addr};
        std::memcpy(
            gpu::buffer_host_address(m_scene_buffer),
            &scene_header,
            sizeof(scene_header));
    }

    Scene::~Scene()
    {
        gpu::destroy_buffer(m_device, m_scene_buffer);
    }

    RenderInstance Scene::create_instance(Mesh mesh, Material material, glm::mat4 transform)
    {
        if (m_instance_slots.size() >= max_instances) {
            throw std::runtime_error("Scene: Maximum instance count exceeded");
        }

        const auto gpu_instance = GpuInstance{
            .mesh_idx     = static_cast<std::uint32_t>(mesh.handle),
            .material_idx = static_cast<std::uint32_t>(material.handle),
            .transform    = transform,
        };
        const auto handle = m_instance_slots.insert({mesh, material, transform});
        const auto offset = sizeof(gpu_instance) * handle.index;
        std::memcpy(
            static_cast<char*>(gpu::buffer_host_address(m_scene_buffer)) + sizeof(GpuSceneHeader) + offset,
            &gpu_instance,
            sizeof(gpu_instance));
        return RenderInstance{handle.to_uint64()};
    }

    void Scene::remove_instance(RenderInstance instance)
    {
        const auto handle = SlotMap<RenderInstanceRecord>::Handle::from_uint64(instance.handle);
        m_instance_slots.erase(handle);
    }
} // namespace orion
