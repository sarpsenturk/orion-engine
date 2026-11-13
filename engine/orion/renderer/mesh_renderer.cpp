#include "mesh_renderer.hpp"

#include "orion/core/logger.hpp"

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace orion
{
    static std::vector<std::uint32_t> load_spirv(const char* path)
    {
        auto file = std::ifstream(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            ORION_LOG_ERROR(LogChannel::Renderer, "Failed to open SPIR-V file: {}", path);
            throw std::runtime_error(fmt::format("Failed to open SPIR-V file: {}", path));
        }
        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<std::uint32_t> spirv(size / sizeof(std::uint32_t));
        file.read(reinterpret_cast<char*>(spirv.data()), size);
        return spirv;
    }

    MeshRenderer::MeshRenderer(gpu::Device* device, const MeshManager& meshes, const MaterialManager& materials,
                               gpu::Format color_format, gpu::Format depth_format)
        : m_device(device)
        , m_meshes(meshes)
        , m_materials(materials)
        , m_pipeline(gpu::create_graphics_pipeline(
              m_device,
              gpu::GraphicsPipelineDesc{
                  .spirv      = load_spirv(ORION_SHADER_OUTPUT_DIR "/mesh.spv"),
                  .topology   = gpu::Topology::TriangleList,
                  .rasterizer = gpu::RasterizerDesc{
                      .cull_mode      = gpu::CullMode::Back,
                      .front_face_ccw = true,
                  },
                  .depth_stencil = gpu::DepthStencilDesc{
                      .depth_test_enable  = true,
                      .depth_write_enable = true,
                      .depth_compare_op   = gpu::CompareOp::LessOrEqual,
                  },
                  .color_attachments = {{color_format}},
                  .depth_attachment  = depth_format,
              }))
    {
    }

    MeshRenderer::~MeshRenderer()
    {
        gpu::wait_idle(m_device);
        gpu::destroy_pipeline(m_device, m_pipeline);
    }

    MeshInstance MeshRenderer::create_instance(Mesh mesh, Material material, glm::mat4 transform)
    {
        if (m_instances.size() >= max_instances) {
            throw std::runtime_error("MeshRenderer: Maximum instance count exceeded");
        }
        const auto handle = m_instances.insert({mesh, material, transform});
        return {handle.to_uint64()};
    }

    void MeshRenderer::remove_instance(MeshInstance instance)
    {
        const auto handle = SlotMap<Instance>::Handle::from_uint64(instance.handle);
        m_instances.erase(handle);
    }

    std::uint32_t MeshRenderer::instance_count() const noexcept
    {
        return static_cast<std::uint32_t>(m_instances.size());
    }

    void MeshRenderer::prepare(Frame& frame, std::span<const ViewIndex> views)
    {
        if (views.empty() || m_instances.size() == 0z) { return; }

        struct alignas(16) GpuInstance {
            std::uint32_t mesh_idx;
            std::uint32_t material_idx;
            std::uint32_t padding[2];
            glm::mat4 transform;
        };
        static_assert(sizeof(GpuInstance) == 80z);
        static_assert(offsetof(GpuInstance, transform) == 16z);

        struct MeshRoot {
            gpu::Address meshes;
            gpu::Address materials;
            gpu::Address view;
            gpu::Address instances;
        };
        static_assert(sizeof(MeshRoot) == 32z);
        static_assert(offsetof(MeshRoot, materials) == 8z);
        static_assert(offsetof(MeshRoot, view) == 16z);
        static_assert(offsetof(MeshRoot, instances) == 24z);

        const auto [buffer, offset] = frame.allocate<GpuInstance>(m_instances.size());
        auto* destination = static_cast<std::byte*>(gpu::buffer_host_address(buffer)) + offset;
        const auto instances_address = gpu::buffer_device_address(buffer) + offset;
        auto instance_index = std::uint32_t{0};
        for (const auto& instance : m_instances.values()) {
            const auto gpu_instance = GpuInstance{
                .mesh_idx     = m_meshes.get_mesh_index(instance.mesh),
                .material_idx = m_materials.get_material_index(instance.material),
                .padding      = {},
                .transform    = instance.transform,
            };
            std::memcpy(destination + sizeof(GpuInstance) * instance_index, &gpu_instance, sizeof(gpu_instance));
            const auto mesh = m_meshes.get_mesh_record(instance.mesh);
            auto packet = DrawPacket{
                .pipeline     = m_pipeline,
                .index_buffer = m_meshes.index_buffer(),
                .draw = {
                    .index_count    = mesh.index_count,
                    .instance_count = 1u,
                    .first_index    = mesh.first_index,
                    .first_instance = instance_index,
                },
            };
            for (const auto view : views) {
                const auto root = MeshRoot{
                    .meshes    = m_meshes.mesh_table_address(),
                    .materials = m_materials.material_table_address(),
                    .view      = frame.view(view).gpu_address,
                    .instances = instances_address,
                };
                packet.set_root(std::as_bytes(std::span{&root, 1z}));
                frame.draw_opaque(view, packet);
            }
            ++instance_index;
        }
    }
} // namespace orion
