#include "orion-renderer/shader.h"

#include "orion-renderapi/render_device.h"

#include "orion-assets/config.h"

#include "orion-utils/assertion.h"
#include "orion-utils/minmax.h"

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace orion
{
    namespace
    {
        std::array<DescriptorLayoutHandle, 4> create_descriptor_layouts(RenderDevice* device, std::span<const ShaderReflection> shader_reflection)
        {
            struct EffectDescriptor {
                const ShaderReflectionDescriptorSet* descriptor;
                ShaderStageFlags stages;
            };
            std::array<EffectDescriptor, 4> descriptor_sets{};
            std::array<DescriptorLayoutHandle, 4> descriptor_layouts{};
            for (const auto& shader : shader_reflection) {
                for (const auto& descriptor : shader.descriptors) {
                    const auto set = descriptor.set;
                    if (set >= 4) {
                        throw std::runtime_error{fmt::format("{}: Shader effects only support up to 4", shader.entry_point)};
                    }
                    if (descriptor_layouts[set] != DescriptorLayoutHandle::invalid()) {
                        if (auto& existing = descriptor_sets[set]; *existing.descriptor != descriptor) {
                            throw std::runtime_error{fmt::format("{}: Previous definition of descriptor set {} does not match current definition", shader.entry_point, set)};
                        } else {
                            existing.stages |= shader.stage;
                        }
                    } else {
                        std::vector<DescriptorBindingDesc> bindings(descriptor.bindings.size());
                        std::ranges::transform(descriptor.bindings, bindings.begin(), [&](const ShaderReflectionDescriptorBinding& binding) {
                            return DescriptorBindingDesc{
                                .type = binding.type,
                                .shader_stages = shader.stage,
                            };
                        });
                        descriptor_sets[set] = {.descriptor = &descriptor, .stages = shader.stage};
                        descriptor_layouts[set] = device->create_descriptor_layout({bindings});
                    }
                }
            }
            return descriptor_layouts;
        }

        std::uint32_t push_constant_size(std::span<const ShaderReflection> shader_reflection)
        {
            std::uint32_t size = 0;
            for (const auto& shader : shader_reflection) {
                for (const auto& push_constant : shader.push_constants) {
                    size = max(size, push_constant.size);
                }
            }
            return size;
        }

        UniquePipelineLayout create_pipeline_layout(RenderDevice* device, std::span<const DescriptorLayoutHandle, 4> descriptor_layouts, std::uint32_t push_constant_size)
        {
            const auto push_constant = std::array{
                PushConstantDesc{
                    .size = push_constant_size,
                    .shader_stages = ShaderStageFlags::All,
                },
            };
            return device->make_unique<PipelineLayoutHandle_tag>(PipelineLayoutDesc{
                .descriptors = descriptor_layouts,
                .push_constants = push_constant_size > 0 ? std::span{push_constant} : std::span<const PushConstantDesc>{},
            });
        }
    } // namespace

    ShaderEffect::ShaderEffect(UniqueShaderModule vertex_shader,
                               UniqueShaderModule pixel_shader,
                               std::array<UniqueDescriptorLayout, 4> descriptor_layouts,
                               UniquePipelineLayout pipeline_layout)
        : vertex_shader_(std::move(vertex_shader))
        , pixel_shader_(std::move(pixel_shader))
        , descriptor_layouts_(std::move(descriptor_layouts))
        , pipeline_layout_(std::move(pipeline_layout))
    {
        ORION_ASSERT(vertex_shader_.get() != ShaderModuleHandle::invalid());
        ORION_ASSERT(pixel_shader_.get() != ShaderModuleHandle::invalid());
    }

    std::array<ShaderStageDesc, 2> ShaderEffect::shader_stages() const
    {
        return {
            ShaderStageDesc{
                .module = vertex_shader(),
                .stage = ShaderStageFlags::Vertex,
                .entry_point = ORION_VS_ENTRY,
            },
            ShaderStageDesc{
                .module = pixel_shader(),
                .stage = ShaderStageFlags::Pixel,
                .entry_point = ORION_PS_ENTRY,
            },
        };
    }

    ShaderEffect create_shader_effect(RenderDevice* device, const FilePath& vs_path, const FilePath& ps_path)
    {
        const auto vs_binary = binary_input_file(vs_path).read_all();
        const auto ps_binary = binary_input_file(ps_path).read_all();

        auto shader_reflector = device->create_shader_reflector();
        const auto shader_reflection = std::array{
            shader_reflector->reflect(vs_binary).value(),
            shader_reflector->reflect(ps_binary).value(),
        };

        const auto descriptor_layouts = create_descriptor_layouts(device, shader_reflection);
        return {
            device->make_unique<ShaderModuleHandle_tag>(ShaderModuleDesc{vs_binary}),
            device->make_unique<ShaderModuleHandle_tag>(ShaderModuleDesc{ps_binary}),
            {device->to_unique(descriptor_layouts[0]), device->to_unique(descriptor_layouts[1]), device->to_unique(descriptor_layouts[2]), device->to_unique(descriptor_layouts[2])},
            create_pipeline_layout(device, descriptor_layouts, push_constant_size(shader_reflection)),
        };
    }
} // namespace orion
