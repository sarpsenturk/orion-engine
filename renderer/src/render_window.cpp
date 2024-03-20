#include "orion-renderer/render_window.h"

#include "orion-renderer/shader.h"

#include "orion-renderer/config.h"

#include "orion-renderapi/render_device.h"

#include "orion-core/window.h"

#include <algorithm>

namespace orion
{
    RenderWindow::RenderWindow(RenderDevice* device, Window* window, CommandAllocator* command_allocator)
        : device_(device)
        , window_(window)
        , swapchain_(device->create_swapchain(*window, swapchain_desc()))
        , present_pass_(create_render_pass())
        , image_views_(create_image_views())
        , framebuffers_(create_framebuffers())
        , descriptor_layout_(create_descriptor_layout())
        , pipeline_layout_(create_pipeline_layout())
        , pipeline_(create_pipeline())
        , sampler_(create_sampler())
        , frames_([this, command_allocator]() { return create_frame_data(command_allocator); })
    {
    }

    void RenderWindow::present(const PresentDesc& desc)
    {
        auto& frame = frames_.get(desc.frame_index);
        auto* command_list = frame.command_list.get();

        // Acquire image from swapchain
        const auto image_index = swapchain_->current_image_index();
        const auto present_size = window_->size();

        // Reset present command list
        device_->wait_for_fence(frame.fence.get());
        command_list->reset();

        // Update descriptor
        const auto image_binding = ImageBindingDesc{
            .image_view_handle = desc.source_image,
            .image_layout = desc.source_image_layout,
        };
        device_->write_descriptor(frame.descriptor.get(), DescriptorBinding{.binding = 0, .binding_type = BindingType::SampledImage, .binding_value = image_binding});

        command_list->begin();
        command_list->begin_render_pass({
            .render_pass = present_pass_.get(),
            .framebuffer = framebuffers_[image_index].get(),
            .render_area = {.size = present_size},
            .clear_color = {},
        });
        command_list->bind_pipeline({.pipeline = pipeline_.get(), .bind_point = PipelineBindPoint::Graphics});
        command_list->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = pipeline_layout_.get(),
            .index = 0,
            .descriptor = frame.descriptor.get(),
        });
        command_list->set_viewports(Viewport{.position = {}, .size = vector_cast<float>(present_size), .depth = {0.f, 1.f}});
        command_list->set_scissors(Scissor{.offset = {}, .size = present_size});
        command_list->draw({.vertex_count = 3, .instance_count = 1, .first_vertex = 0, .first_instance = 0});
        command_list->end_render_pass();
        command_list->end();

        device_->submit(
            {
                .queue_type = CommandQueueType::Graphics,
                .wait_semaphores = desc.wait_semaphores,
                .command_lists = {{command_list}},
                .signal_semaphores = {{frame.semaphore.get()}},
            },
            frame.fence.get());

        swapchain_->present({{frame.semaphore.get()}});
    }

    SwapchainDesc RenderWindow::swapchain_desc() const noexcept
    {
        return {
            .image_count = frames_in_flight,
            .image_format = Format::B8G8R8A8_Srgb,
            .image_size = window_->size(),
            .image_usage = ImageUsageFlags::ColorAttachment,
        };
    }

    UniqueRenderPass RenderWindow::create_render_pass() const
    {
        return device_->make_unique<RenderPassHandle_tag>(RenderPassDesc{
            .color_attachments = {{
                AttachmentDesc{
                    .format = Format::B8G8R8A8_Srgb,
                    .load_op = AttachmentLoadOp::DontCare,
                    .store_op = AttachmentStoreOp::Store,
                    .initial_layout = ImageLayout::Undefined,
                    .layout = ImageLayout::ColorAttachment,
                    .final_layout = ImageLayout::PresentSrc,
                },
            }},
            .bind_point = PipelineBindPoint::Graphics,
        });
    }

    std::vector<UniqueImageView> RenderWindow::create_image_views() const
    {
        std::vector<UniqueImageView> image_views(frames_in_flight);
        std::ranges::generate(image_views, [this, index = 0u]() mutable {
            return device_->make_unique<ImageViewHandle_tag>(ImageViewDesc{
                .image = swapchain_->get_image(index++),
                .type = ImageViewType::View2D,
                .format = Format::B8G8R8A8_Srgb,
            });
        });
        return image_views;
    }

    std::vector<UniqueFramebuffer> RenderWindow::create_framebuffers() const
    {
        std::vector<UniqueFramebuffer> framebuffers(frames_in_flight);
        std::ranges::generate(framebuffers, [&, index = 0u]() mutable {
            return device_->make_unique<FramebufferHandle_tag>(FramebufferDesc{
                .render_pass = present_pass_.get(),
                .image_views = {{image_views_[index++].get()}},
                .size = window_->size(),
            });
        });
        return framebuffers;
    }

    UniqueDescriptorLayout RenderWindow::create_descriptor_layout() const
    {
        return device_->make_unique<DescriptorLayoutHandle_tag>(DescriptorLayoutDesc{
            .bindings = {{
                DescriptorBindingDesc{
                    .type = BindingType::SampledImage,
                    .shader_stages = ShaderStageFlags::Pixel,
                    .count = 1,
                },
                DescriptorBindingDesc{
                    .type = BindingType::Sampler,
                    .shader_stages = ShaderStageFlags::Pixel,
                    .count = 1,
                },
            }},
        });
    }

    UniquePipelineLayout RenderWindow::create_pipeline_layout() const
    {
        return device_->make_unique<PipelineLayoutHandle_tag>(PipelineLayoutDesc{
            .descriptors = {{descriptor_layout_.get()}},
        });
    }

    UniquePipeline RenderWindow::create_pipeline() const
    {
        auto shaders = ShaderManager{device_};
        const auto shader_effect = shaders.load_shader_effect("present");
        return device_->make_unique<PipelineHandle_tag>(GraphicsPipelineDesc{
            .shaders = shader_effect.shader_stages(),
            .pipeline_layout = pipeline_layout_.get(),
            .rasterization = {.cull_mode = CullMode::Back, .front_face = FrontFace::ClockWise},
            .color_blend = {
                .attachments = {{
                    BlendAttachmentDesc{
                        .enable_blend = true,
                        .src_blend = BlendFactor::One,
                        .dst_blend = BlendFactor::Zero,
                        .blend_op = BlendOp::Add,
                        .color_component_flags = ColorComponentFlags::All,
                    },
                }},
            },
            .render_pass = present_pass_.get(),
        });
    }

    UniqueSampler RenderWindow::create_sampler() const
    {
        return device_->make_unique<SamplerHandle_tag>(SamplerDesc{
            .filter = Filter::Nearest,
            .address_mode_u = AddressMode::Repeat,
            .address_mode_v = AddressMode::Repeat,
            .address_mode_w = AddressMode::Repeat,
            .mip_load_bias = 0.f,
            .max_anisotropy = 0.f,
            .compare_func = CompareFunc::Always,
            .min_lod = 0.f,
            .max_lod = 0.f,
        });
    }

    RenderWindow::FrameData RenderWindow::create_frame_data(CommandAllocator* command_allocator) const
    {
        auto descriptor = device_->make_unique<DescriptorHandle_tag>(descriptor_layout_.get());
        device_->write_descriptor(descriptor.get(), DescriptorBinding{.binding = 1, .binding_type = BindingType::Sampler, .binding_value = ImageBindingDesc{.sampler_handle = sampler_.get()}});
        return {
            .command_list = command_allocator->create_command_list(),
            .descriptor = std::move(descriptor),
            .fence = device_->make_unique<FenceHandle_tag>(FenceDesc{.start_finished = true}),
            .semaphore = device_->make_unique<SemaphoreHandle_tag>(),
        };
    }
} // namespace orion