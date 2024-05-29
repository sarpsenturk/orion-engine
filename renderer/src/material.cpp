#include "orion-renderer/material.h"

#include "orion-renderer/render_context.h"

#include "orion-renderapi/render_device.h"

#include <array>
#include <bit>
#include <utility>

namespace orion
{
    namespace
    {
        UniqueDescriptorPool create_descriptor_pool(RenderDevice* device)
        {
            return device->make_unique<DescriptorPoolHandle_tag>(DescriptorPoolDesc{
                .max_descriptors = MaterialBuilder::max_materials,
                .flags = {},
                .sizes = {{
                    DescriptorPoolSize{
                        .type = DescriptorType::ConstantBuffer,
                        .count = MaterialBuilder::max_materials,
                    },
                }},
            });
        }
    } // namespace

    Material::Material(const Effect* effect, UniqueGPUBuffer constant_buffer, UniqueDescriptor descriptor, MaterialData data)
        : effect_(effect)
        , constant_buffer_(std::move(constant_buffer))
        , descriptor_(std::move(descriptor))
        , material_data_(data)
    {
    }

    MaterialBuilder::MaterialBuilder(RenderContext* context, DescriptorLayoutHandle material_layout)
        : context_(context)
        , material_layout_(material_layout)
        , descriptor_pool_(create_descriptor_pool(context->device()))
    {
    }

    Material MaterialBuilder::create(const Effect* effect, const MaterialData& data)
    {
        const auto buffer_size = sizeof(MaterialData);
        auto* device = context_->device();
        auto constant_buffer = device->create_buffer({
            .size = buffer_size,
            .usage = GPUBufferUsageFlags::ConstantBuffer | GPUBufferUsageFlags::TransferDst,
            .host_visible = true,
        });

        // Upload material data to GPU
        {
            const auto bytes = std::bit_cast<std::array<std::byte, sizeof(MaterialData)>>(data);
            context_->copy_buffer_staging({.bytes = bytes, .dst = constant_buffer});
        }

        auto descriptor = device->create_descriptor(material_layout_, descriptor_pool_.get());
        // Write to descriptor
        {
            const auto buffer_write = BufferDescriptorDesc{.buffer_handle = constant_buffer, .region = {.size = buffer_size, .offset = 0}};
            const auto write = DescriptorWrite{
                .binding = 0,
                .descriptor_type = DescriptorType::ConstantBuffer,
                .array_start = 0,
                .buffers = {&buffer_write, 1},
            };
            device->write_descriptor(descriptor, write);
        }

        return Material{effect, device->to_unique(constant_buffer), device->to_unique(descriptor), data};
    }
} // namespace orion
