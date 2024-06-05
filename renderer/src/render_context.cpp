#include "orion-renderer/render_context.h"

#include "orion-renderer/types.h"

#include "orion-renderapi/render_device.h"

#include <cstring>

// TODO:
//  staging buffer copies should be done via a function which can handle
//  copying of data larger than the staging buffer by dividing copies.
//  Currently attempting to stage any data larger than staging_buffer_size
//  will simply fail.

namespace orion
{
    namespace
    {
        constexpr auto staging_buffer_size = 1024 * 1024;
        constexpr auto object_buffer_size = sizeof(RenderObjBuffer) * 1000;

        DescriptorPoolHandle create_descriptor_pool(RenderDevice* device)
        {
            return device->create_descriptor_pool({
                .max_descriptors = 1 + frames_in_flight * 2,
                .flags = {},
                .sizes = {{
                    DescriptorPoolSize{
                        .type = DescriptorType::Sampler,
                        .count = 1,
                    },
                    DescriptorPoolSize{
                        .type = DescriptorType::SampledImage,
                        .count = 1,
                    },
                    DescriptorPoolSize{
                        .type = DescriptorType::ConstantBuffer,
                        .count = frames_in_flight,
                    },
                    DescriptorPoolSize{
                        .type = DescriptorType::StorageBuffer,
                        .count = frames_in_flight,
                    },
                }},
            });
        }

        void init_present_descriptor(RenderDevice* device, DescriptorHandle present_descriptor, ImageViewHandle render_image, SamplerHandle sampler)
        {
            const auto render_output_image = ImageDescriptorDesc{
                .image_view_handle = render_image,
                .image_layout = ImageLayout::ShaderReadOnly,
            };
            const auto render_output_sampler = ImageDescriptorDesc{
                .sampler_handle = sampler,
            };
            const auto descriptor_writes = std::array{
                DescriptorWrite{
                    .binding = 0,
                    .descriptor_type = DescriptorType::SampledImage,
                    .images = {&render_output_image, 1},
                },
                DescriptorWrite{
                    .binding = 1,
                    .descriptor_type = DescriptorType::Sampler,
                    .images = {&render_output_sampler, 1},
                },
            };
            device->write_descriptor(present_descriptor, descriptor_writes);
        }

        DescriptorHandle create_frame_descriptor(RenderDevice* device, DescriptorLayoutHandle layout, DescriptorPoolHandle descriptor_pool, GPUBufferHandle cbuffer)
        {
            const auto descriptor = device->create_descriptor(layout, descriptor_pool);
            const auto buffer_write = BufferDescriptorDesc{
                .buffer_handle = cbuffer,
                .region = {
                    .size = sizeof(SceneCBuffer),
                    .offset = 0,
                },
            };
            const auto write = DescriptorWrite{
                .binding = 0,
                .descriptor_type = DescriptorType::ConstantBuffer,
                .array_start = 0,
                .buffers = {&buffer_write, 1},
            };
            device->write_descriptor(descriptor, write);
            return descriptor;
        }

        DescriptorHandle create_object_descriptor(RenderDevice* device, DescriptorLayoutHandle layout, DescriptorPoolHandle descriptor_pool, GPUBufferHandle buffer)
        {
            const auto descriptor = device->create_descriptor(layout, descriptor_pool);
            const auto buffer_write = BufferDescriptorDesc{
                .buffer_handle = buffer,
                .region = {
                    .size = object_buffer_size,
                    .offset = 0,
                },
            };
            const auto write = DescriptorWrite{
                .binding = 0,
                .descriptor_type = DescriptorType::StorageBuffer,
                .array_start = 0,
                .buffers = {&buffer_write, 1},
            };
            device->write_descriptor(descriptor, write);
            return descriptor;
        }

        FrameInFlight create_frame(RenderDevice* device, const FrameInFlightDesc& desc)
        {
            const auto render_target_desc = RenderTargetDesc{
                .size = desc.render_size,
                .image_usage = ImageUsageFlags::ColorAttachment | ImageUsageFlags::SampledImage,
                .initial_layout = ImageLayout::Undefined,
                .final_layout = ImageLayout::ShaderReadOnly,
            };
            auto render_target = create_render_target(device, render_target_desc);

            auto descriptor_pool = create_descriptor_pool(device);
            auto render_output_descriptor = device->create_descriptor(desc.present_descriptor_layout, descriptor_pool);
            init_present_descriptor(device, render_output_descriptor, render_target.image_view(), desc.present_sampler);

            auto command_allocator = device->create_command_allocator({.queue_type = CommandQueueType::Graphics, .reset_command_buffer = false});
            auto render_command = command_allocator->create_command_list();
            auto present_command = command_allocator->create_command_list();

            auto staging_buffer = device->create_buffer({.size = staging_buffer_size, .usage = GPUBufferUsageFlags::TransferSrc, .host_visible = true});

            auto frame_cbuffer = device->create_buffer({.size = sizeof(SceneCBuffer), .usage = GPUBufferUsageFlags::ConstantBuffer, .host_visible = true});
            auto frame_descriptor = create_frame_descriptor(device, desc.frame_descriptor_layout, descriptor_pool, frame_cbuffer);

            auto object_buffer = device->create_buffer({.size = object_buffer_size, .usage = GPUBufferUsageFlags::StorageBuffer, .host_visible = true});
            auto object_descriptor = create_object_descriptor(device, desc.object_descriptor_layout, descriptor_pool, object_buffer);

            return FrameInFlight{
                .command_allocator = std::move(command_allocator),
                .descriptor_pool = descriptor_pool,
                .render_command = std::move(render_command),
                .render_fence = device->create_fence({.start_finished = false}),
                .render_semaphore = device->create_semaphore(),
                .render_target = std::move(render_target),
                .render_output_descriptor = render_output_descriptor,
                .present_command = std::move(present_command),
                .present_fence = device->create_fence({.start_finished = true}),
                .present_semaphore = device->create_semaphore(),
                .staging_buffer = staging_buffer,
                .frame_cbuffer = frame_cbuffer,
                .frame_descriptor = frame_descriptor,
                .object_buffer = object_buffer,
                .object_descriptor = object_descriptor,
            };
        }

        static_vector<FrameInFlight, frames_in_flight> create_frames_in_flight(RenderDevice* device, const FrameInFlightDesc& desc)
        {
            static_vector<FrameInFlight, frames_in_flight> frames;
            for (int i = 0; i < frames_in_flight; ++i) {
                frames.push_back(create_frame(device, desc));
            }
            return frames;
        }
    } // namespace

    RenderContext::RenderContext(RenderDevice* device, const FrameInFlightDesc& desc)
        : device_(device)
        , frames_(create_frames_in_flight(device, desc))
    {
    }

    void RenderContext::advance_frame()
    {
        previous_frame_index_ = current_frame_index_;
        current_frame_index_ = (current_frame_index_ + 1) % frames_in_flight;
    }

    void RenderContext::copy_buffer_staging(const CopyBufferStaging& copy)
    {
        copy_buffer_staging({&copy, 1});
    }

    void RenderContext::copy_buffer_staging(std::span<const CopyBufferStaging> copies)
    {
        auto command_list = command_allocator()->create_command_list();
        command_list->begin();
        const auto src_buffer = staging_buffer();
        // TODO: Persistent mapping could be a better option here
        void* base = device_->map(staging_buffer());
        // TODO: Assert size of copies is less than staging_buffer_size
        //  or copy staging_buffer_size bytes at a time
        for (std::size_t offset = 0; const auto& copy : copies) {
            std::memcpy(static_cast<char*>(base) + offset, copy.bytes.data(), copy.bytes.size_bytes());
            command_list->copy_buffer({.src = src_buffer, .dst = copy.dst, .src_offset = offset, .dst_offset = copy.dst_offset, .size = copy.bytes.size_bytes()});
            offset += copy.bytes.size_bytes();
        }
        device_->unmap(staging_buffer());
        command_list->end();
        device_->submit_immediate({.queue_type = CommandQueueType::Graphics, .command_lists = {{command_list.get()}}});
    }

    void RenderContext::copy_image_staging(const CopyImageStaging& copy)
    {
        auto cmd_list = command_allocator()->create_command_list();
        cmd_list->begin();
        cmd_list->transition_barrier({.image = copy.dst, .old_layout = copy.dst_initial_layout, .new_layout = ImageLayout::TransferDst});
        const auto staging = staging_buffer();
        // Upload image data to staging buffer
        {
            void* dst = device_->map(staging);
            std::memcpy(dst, copy.bytes.data(), copy.bytes.size_bytes());
            device_->unmap(staging);
        }
        cmd_list->copy_buffer_to_image({
            .src_buffer = staging,
            .dst_image = copy.dst,
            .dst_layout = ImageLayout::TransferDst,
            .buffer_offset = 0,
            .image_offset = 0,
            .dst_size = copy.dst_size,
        });
        cmd_list->transition_barrier({.image = copy.dst, .old_layout = ImageLayout::TransferDst, .new_layout = copy.dst_final_layout});
        cmd_list->end();
        device_->submit_immediate({.queue_type = CommandQueueType::Graphics, .command_lists = {{cmd_list.get()}}});
    }

    void RenderContext::memcpy(ImageHandle image, std::span<const std::byte> bytes, std::size_t offset)
    {
        void* dst = device_->map(image);
        std::memcpy(dst, bytes.data(), bytes.size_bytes());
        device_->unmap(image);
    }
} // namespace orion
