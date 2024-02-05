#include "orion-renderer/renderer.h"

#include "orion-renderer/colors.h"

#include "imgui_impl_orion.h"
#include <imgui.h>

#include "orion-renderapi/config.h"

#include "orion-core/window.h"

#include "orion-utils/assertion.h"

#include "orion-scene/components.h"
#include "orion-scene/scene.h"

#include <algorithm>

#ifndef ORION_RENDERER_LOG_LEVEL
    #define ORION_RENDERER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    const char* default_backend_module(Platform platform)
    {
        switch (platform) {
            case Platform::Unknown:
                break;
            case Platform::Windows:
                return ORION_VULKAN_MODULE;
            case Platform::Linux:
                break;
        }
        ORION_ASSERT(!"Current platform is not supported by the renderer");
        return nullptr;
    }

    ImGuiContext::~ImGuiContext()
    {
        ImGui_ImplOrion_Shutdow();
        ImGui::DestroyContext();
    }

    Renderer::Renderer(const RendererDesc& desc)
        : backend_module_(desc.backend_module)
        , render_backend_(create_render_backend())
        , render_device_(create_render_device(desc.device_select_fn))
        , render_pass_(create_render_pass())
        , present_pass_(create_present_pass())
        , swapchain_(create_swapchain(desc.window))
        , swapchain_image_views_(create_swapchain_image_views())
        , swapchain_framebuffers_(create_swapchain_framebuffers(desc.window->size()))
        , command_allocator_(create_command_allocator())
        , shader_manager_(device())
        , render_size_(desc.render_size)
        , viewport_(Vector2_f{}, vector_cast<float>(render_size_), {0.f, 1.f})
        , scissor_({}, render_size_)
        , triangle_pipeline_layout_(create_triangle_pipeline_layout())
        , triangle_shader_effect_(shader_manager_.load_shader_effect("triangle"))
        , triangle_pipeline_(create_triangle_pipeline())
        , present_descriptor_layout_(create_present_descriptor_layout())
        , present_pipeline_layout_(create_present_pipeline_layout())
        , present_sampler_(create_present_sampler())
        , present_shader_effect_(shader_manager_.load_shader_effect("present"))
        , present_pipeline_(create_present_pipeline())
        , frames_([this] { return create_frame_data(); })
        , imgui_(desc.init_imgui ? create_imgui_context(desc.window) : nullptr)
    {
    }

    QuadRenderer Renderer::create_quad_renderer()
    {
        return {device(), &shader_manager_, render_pass_};
    }

    void Renderer::begin()
    {
        auto& frame = frames_.current_frame();

        // Wait for frame to finish
        device()->wait_for_fence(frame.fence);

        // Reset command list
        frame.command_list->reset();

        // Begin command list
        frame.command_list->begin();

        // Begin render pass
        frame.command_list->begin_render_pass({
            .render_pass = render_pass_,
            .framebuffer = frame.render_target,
            .render_area = {
                .offset = {0, 0},
                .size = render_size_,
            },
            .clear_color = colors::magenta,
        });

        // Set the viewports
        frame.command_list->set_viewports(viewport_);

        // Set the scissors
        frame.command_list->set_scissors(scissor_);
    }

    void Renderer::end()
    {
        auto& frame = frames_.current_frame();

        // Finish render pass
        frame.command_list->end_render_pass();

        // End command list recording
        frame.command_list->end();

        // Submit command buffer
        device()->submit(
            {
                .queue_type = CommandQueueType::Graphics,
                .command_lists = {{frame.command_list.get()}},
                .signal_semaphores = {{frame.render_semaphore}},
            },
            frame.fence);

        // Advance to next frame
        advance_frame();
    }

    void Renderer::imgui_begin()
    {
        ORION_ASSERT(imgui_ && "ImGui not initialized. RendererDesc::init_imgui must be true to use ImGui");
        ImGui_ImplOrion_NewFrame(current_frame_index_);
        ImGui::NewFrame();
    }

    void Renderer::imgui_end()
    {
        ORION_ASSERT(imgui_ && "ImGui not initialized. RendererDesc::init_imgui must be true to use ImGui");
        ImGui::Render();
        ImGui_ImplOrion_RenderDrawData(ImGui::GetDrawData(), frames_.current_frame().command_list.get());
    }

    void Renderer::render(QuadRenderer& quad_renderer)
    {
        auto* command_list = frames_.current_frame().command_list.get();

        quad_renderer.flush(command_list);
    }

    void Renderer::draw_test_triangle()
    {
        auto& frame = frames_.current_frame();

        // Bind the triangle pipeline
        frame.command_list->bind_pipeline({.pipeline = triangle_pipeline_, .bind_point = PipelineBindPoint::Graphics});

        // Make draw call
        frame.command_list->draw({.vertex_count = 3, .instance_count = 1, .first_vertex = 0, .first_instance = 0});
    }

    void Renderer::present(const Window& window)
    {
        auto& frame = frames_.previous_frame();

        auto* command_list = frame.present_command.get();

        // Acquire image from swapchain
        const auto image_index = swapchain_->current_image_index();
        const auto present_size = window.size();

        // Reset present command list
        device()->wait_for_fence(frame.present_fence);
        command_list->reset();

        command_list->begin();
        command_list->begin_render_pass({
            .render_pass = present_pass_,
            .framebuffer = swapchain_framebuffers_[image_index],
            .render_area = {.size = present_size},
            .clear_color = {},
        });
        command_list->bind_pipeline({.pipeline = present_pipeline_, .bind_point = PipelineBindPoint::Graphics});
        command_list->bind_descriptor({
            .bind_point = PipelineBindPoint::Graphics,
            .pipeline_layout = present_pipeline_layout_,
            .index = 0,
            .descriptor = frame.present_descriptor,
        });
        command_list->set_viewports(Viewport{.position = {}, .size = vector_cast<float>(present_size), .depth = {0.f, 1.f}});
        command_list->set_scissors(Scissor{.offset = {}, .size = present_size});
        command_list->draw({.vertex_count = 3, .instance_count = 1, .first_vertex = 0, .first_instance = 0});
        command_list->end_render_pass();
        command_list->end();

        device()->submit(
            {
                .queue_type = CommandQueueType::Graphics,
                .wait_semaphores = {{frame.render_semaphore}},
                .command_lists = {{command_list}},
                .signal_semaphores = {{frame.present_semaphore}},
            },
            frame.present_fence);

        swapchain_->present({{frame.present_semaphore}});
    }

    spdlog::logger* Renderer::logger()
    {
        static const auto logger = create_logger("orion-renderer", ORION_RENDERER_LOG_LEVEL);
        return logger.get();
    }

    std::unique_ptr<RenderBackend> Renderer::create_render_backend() const
    {
        ORION_ASSERT(backend_module_.is_loaded());

        auto* fn_create_render_backend = backend_module_.load_symbol<CreateOrionRenderBackendFn>("create_orion_render_backend");
        if (fn_create_render_backend == nullptr) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to load function 'create_orion_render_backend' from module {}", backend_module_.filename());
            throw std::runtime_error("failed to load create_orion_render_backend()");
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Symbol create_orion_render_backend() loaded at {}", fmt::ptr(fn_create_render_backend));

        RenderBackend* render_backend = fn_create_render_backend();
        if (render_backend == nullptr) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to create render backend");
            throw std::runtime_error("failed to create render backend");
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Render backend created");

        return std::unique_ptr<RenderBackend>{render_backend};
    }

    std::unique_ptr<RenderDevice> Renderer::create_render_device(SelectPhysicalDeviceFn device_select_fn) const
    {
        ORION_ASSERT(render_backend_ != nullptr);

        const auto physical_devices = render_backend_->enumerate_physical_devices();
        if (physical_devices.empty()) {
            throw std::runtime_error("no physical devices found");
        }
        SPDLOG_LOGGER_INFO(logger(), "Found {} physical device(s):", physical_devices.size());
        for (const auto& device : physical_devices) {
            SPDLOG_LOGGER_INFO(logger(), "{}", device.name);
            SPDLOG_LOGGER_INFO(logger(), "-- Index: {}", device.index);
            SPDLOG_LOGGER_INFO(logger(), "-- Type: {}", device.type);
        }

        const auto physical_device_index = [&]() {
            if (device_select_fn != nullptr) {
                return device_select_fn(physical_devices);
            }
            return physical_devices[0].index;
        }();

        if (physical_device_index == invalid_physical_device_index) {
            throw std::runtime_error("no suitable physical device found");
        }

        SPDLOG_LOGGER_INFO(logger(), "Using physical device with index {}", physical_device_index);

        auto render_device = render_backend_->create_device(physical_device_index);
        SPDLOG_LOGGER_DEBUG(logger(), "Render device created");
        return render_device;
    }

    std::unique_ptr<Swapchain> Renderer::create_swapchain(const Window* window) const
    {
        ORION_ASSERT(window != nullptr);
        ORION_ASSERT(device() != nullptr);
        const auto desc = SwapchainDesc{
            .image_count = swapchain_image_count,
            .image_format = swapchain_image_format,
            .image_size = window->size(),
            .image_usage = ImageUsageFlags::ColorAttachment,
        };
        return device()->create_swapchain(*window, desc);
    }
    std::vector<ImageViewHandle> Renderer::create_swapchain_image_views() const
    {
        std::vector<ImageViewHandle> image_views(swapchain_image_count);
        std::ranges::generate(image_views, [this, index = 0u]() mutable {
            return device()->create_image_view({
                .image = swapchain_->get_image(index++),
                .type = ImageViewType::View2D,
                .format = swapchain_image_format,
            });
        });
        return image_views;
    }
    std::vector<FramebufferHandle> Renderer::create_swapchain_framebuffers(const Vector2_u& size) const
    {
        std::vector<FramebufferHandle> framebuffers(swapchain_image_count);
        std::ranges::generate(framebuffers, [&, index = 0u]() mutable {
            return device()->create_framebuffer({
                .render_pass = present_pass_,
                .image_views = {{swapchain_image_views_[index++]}},
                .size = size,
            });
        });
        return framebuffers;
    }

    std::unique_ptr<CommandAllocator> Renderer::create_command_allocator() const
    {
        return device()->create_command_allocator({
            .queue_type = CommandQueueType::Graphics,
            .reset_command_buffer = true,
        });
    }

    RenderPassHandle Renderer::create_render_pass() const
    {
        return device()->create_render_pass({
            .color_attachments = {{
                AttachmentDesc{
                    .format = Format::B8G8R8A8_Srgb,
                    .load_op = AttachmentLoadOp::Clear,
                    .store_op = AttachmentStoreOp::Store,
                    .initial_layout = ImageLayout::Undefined,
                    .layout = ImageLayout::ColorAttachment,
                    .final_layout = ImageLayout::ShaderReadOnly,
                },
            }},
            .input_attachments = {},
            .bind_point = PipelineBindPoint::Graphics,
        });
    }

    PipelineLayoutHandle Renderer::create_triangle_pipeline_layout() const
    {
        return device()->create_pipeline_layout({.descriptors = {}, .push_constants = {}});
    }

    PipelineHandle Renderer::create_triangle_pipeline() const
    {
        return device()->create_graphics_pipeline({
            .shaders = triangle_shader_effect_.shader_stages(),
            .vertex_bindings = {},
            .pipeline_layout = triangle_pipeline_layout_,
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
            .render_pass = render_pass_,
        });
    }

    DescriptorLayoutHandle Renderer::create_present_descriptor_layout() const
    {
        return device()->create_descriptor_layout({
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

    PipelineLayoutHandle Renderer::create_present_pipeline_layout() const
    {
        return device()->create_pipeline_layout({
            .descriptors = {{present_descriptor_layout_}},
        });
    }

    SamplerHandle Renderer::create_present_sampler() const
    {
        return device()->create_sampler({
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

    RenderPassHandle Renderer::create_present_pass() const
    {
        return device()->create_render_pass({
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

    PipelineHandle Renderer::create_present_pipeline() const
    {
        return device()->create_graphics_pipeline({
            .shaders = present_shader_effect_.shader_stages(),
            .pipeline_layout = present_pipeline_layout_,
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
            .render_pass = present_pass_,
        });
    }

    Renderer::FrameData Renderer::create_frame_data() const
    {
        auto image = device()->create_image({
            .type = ImageType::Image2D,
            .format = Format::B8G8R8A8_Srgb,
            .size = vec3(render_size_, 1u),
            .tiling = ImageTiling::Optimal,
            .usage = ImageUsageFlags::ColorAttachment | ImageUsageFlags::SampledImage | ImageUsageFlags::TransferSrc,
        });
        auto image_view = device()->create_image_view({
            .image = image,
            .type = ImageViewType::View2D,
            .format = Format::B8G8R8A8_Srgb,
        });
        auto render_target = device()->create_framebuffer({
            .render_pass = render_pass_,
            .image_views = {&image_view, 1},
            .size = render_size_,
        });

        // Create and update presentation descriptor
        auto present_descriptor = device()->create_descriptor(present_descriptor_layout_);
        const auto present_bindings = std::array{
            DescriptorBinding{
                .binding = 0,
                .binding_type = BindingType::SampledImage,
                .binding_value = ImageBindingDesc{.image_view_handle = image_view, .image_layout = ImageLayout::ShaderReadOnly},
            },
            DescriptorBinding{
                .binding = 1,
                .binding_type = BindingType::Sampler,
                .binding_value = ImageBindingDesc{.sampler_handle = present_sampler_},
            },
        };
        device()->write_descriptor(present_descriptor, present_bindings);

        auto command_list = command_allocator_->create_command_list();
        auto present_command = command_allocator_->create_command_list();
        return {
            .render_image = image,
            .render_image_view = image_view,
            .render_target = render_target,
            .command_list = std::move(command_list),
            .present_command = std::move(present_command),
            .fence = device()->create_fence({.start_finished = true}),
            .render_semaphore = device()->create_semaphore(),
            .present_semaphore = device()->create_semaphore(),
            .present_fence = device()->create_fence({.start_finished = true}),
            .present_descriptor = present_descriptor,
        };
    }

    std::unique_ptr<ImGuiContext> Renderer::create_imgui_context(Window* window)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

        // Setup Platform/Renderer backends
        ImGui_ImplOrion_Init({
            .window = window,
            .device = device(),
            .command_allocator = command_allocator_.get(),
            .render_pass = render_pass_,
            .shader_manager = &shader_manager_,
        });
        SPDLOG_LOGGER_DEBUG(logger(), "Renderer ImGui initialized");
        return std::make_unique<ImGuiContext>();
    }
} // namespace orion
