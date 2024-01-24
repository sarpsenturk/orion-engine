#include "imgui_impl_orion.h"

#ifndef ORION_IMGUI_LOG_LEVEL
    #define ORION_IMGUI_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

#include <array>

#include "orion-math/matrix/matrix4.h"
#include "orion-math/matrix/projection.h"

#include "orion-utils/minmax.h"

#include "orion-core/clock.h"
#include "orion-core/input.h"
#include "orion-core/window.h"

#include "orion-renderapi/buffer.h"
#include "orion-renderapi/render_device.h"

#include "orion-renderer/config.h"
#include "orion-renderer/shader.h"

namespace
{
    struct ImGuiCSceneBuffer {
        orion::Matrix4_f projection;
    };

    auto* logger()
    {
        static const auto logger = orion::create_logger("orion-imgui", ORION_IMGUI_LOG_LEVEL);
        return logger.get();
    }

    ImGuiKey to_imgui_key(orion::KeyCode key_code)
    {
        switch (key_code) {
            case orion::KeyCode::Backspace:
                return ImGuiKey_Backspace;
            case orion::KeyCode::Tab:
                return ImGuiKey_Tab;
            case orion::KeyCode::Enter:
                return ImGuiKey_Enter;
            case orion::KeyCode::Escape:
                return ImGuiKey_Escape;
            case orion::KeyCode::Space:
                return ImGuiKey_Space;
            case orion::KeyCode::Quote:
                return ImGuiKey_Apostrophe;
            case orion::KeyCode::Comma:
                return ImGuiKey_Comma;
            case orion::KeyCode::Minus:
                return ImGuiKey_Minus;
            case orion::KeyCode::Period:
                return ImGuiKey_Period;
            case orion::KeyCode::Slash:
                return ImGuiKey_Slash;
            case orion::KeyCode::Semicolon:
                return ImGuiKey_Semicolon;
            case orion::KeyCode::Equal:
                return ImGuiKey_Equal;
            case orion::KeyCode::LeftBracket:
                return ImGuiKey_LeftBracket;
            case orion::KeyCode::Backslash:
                return ImGuiKey_Backslash;
            case orion::KeyCode::RightBracket:
                return ImGuiKey_RightBracket;
            case orion::KeyCode::Backtick:
                return ImGuiKey_GraveAccent;
            case orion::KeyCode::Delete:
                return ImGuiKey_Delete;
            case orion::KeyCode::Insert:
                return ImGuiKey_Insert;
            case orion::KeyCode::Home:
                return ImGuiKey_Home;
            case orion::KeyCode::End:
                return ImGuiKey_End;
            case orion::KeyCode::PageUp:
                return ImGuiKey_PageUp;
            case orion::KeyCode::PageDown:
                return ImGuiKey_PageDown;
            case orion::KeyCode::PrintScreen:
                return ImGuiKey_PrintScreen;
            case orion::KeyCode::ScrollLock:
                return ImGuiKey_ScrollLock;
            case orion::KeyCode::Pause:
                return ImGuiKey_Pause;
            case orion::KeyCode::Caps:
                return ImGuiKey_CapsLock;
            case orion::KeyCode::Shift:
                return ImGuiKey_LeftShift;
            case orion::KeyCode::Control:
                return ImGuiKey_LeftCtrl;
            case orion::KeyCode::Alt:
                return ImGuiKey_LeftAlt;
            case orion::KeyCode::Command:
                return ImGuiKey_LeftSuper;
            case orion::KeyCode::NumDivide:
                return ImGuiKey_KeypadDivide;
            case orion::KeyCode::NumMultiply:
                return ImGuiKey_KeypadMultiply;
            case orion::KeyCode::NumMinus:
                return ImGuiKey_KeypadSubtract;
            case orion::KeyCode::NumPlus:
                return ImGuiKey_KeypadAdd;
            case orion::KeyCode::NumEnter:
                return ImGuiKey_KeypadEnter;
            case orion::KeyCode::NumPeriod:
                return ImGuiKey_KeypadDecimal;
            case orion::KeyCode::NumLock:
                return ImGuiKey_NumLock;
            case orion::KeyCode::LeftArrow:
                return ImGuiKey_LeftArrow;
            case orion::KeyCode::UpArrow:
                return ImGuiKey_UpArrow;
            case orion::KeyCode::RightArrow:
                return ImGuiKey_RightArrow;
            case orion::KeyCode::DownArrow:
                return ImGuiKey_DownArrow;
            default:
                if (orion::is_numeric_key(key_code)) {
                    const auto offset = orion::to_underlying(key_code) - orion::to_underlying(orion::KeyCode::Alpha0);
                    return static_cast<ImGuiKey>(ImGuiKey_0 + offset);
                } else if (orion::is_char_key(key_code)) {
                    const auto offset = orion::to_underlying(key_code) - orion::to_underlying(orion::KeyCode::KeyA);
                    return static_cast<ImGuiKey>(ImGuiKey_A + offset);
                } else if (orion::is_fn_key(key_code)) {
                    const auto offset = orion::to_underlying(key_code) - orion::to_underlying(orion::KeyCode::F1);
                    constexpr auto imgui_max_fn_offset = 11;
                    if (offset <= imgui_max_fn_offset) {
                        return static_cast<ImGuiKey>(ImGuiKey_F1 + offset);
                    }
                } else if (orion::is_numpad_key(key_code)) {
                    const auto offset = orion::to_underlying(key_code) - orion::to_underlying(orion::KeyCode::Num0);
                    return static_cast<ImGuiKey>(ImGuiKey_Keypad0 + offset);
                }
        }
        return static_cast<ImGuiKey>(-1);
    }

    ImGuiMouseButton to_imgui_mouse_button(orion::MouseButton button)
    {
        switch (button) {
            case orion::MouseButton::Left:
                return ImGuiMouseButton_Left;
            case orion::MouseButton::Right:
                return ImGuiMouseButton_Right;
            case orion::MouseButton::Middle:
                return ImGuiMouseButton_Middle;
            default:
                break;
        }
        return -1;
    }

    void imgui_on_resize(const orion::events::WindowResizeEnd& resize_end)
    {
        ImGui::GetIO().DisplaySize = resize_end.size;
    }

    void imgui_on_key(orion::KeyCode key, bool down)
    {
        if (const auto imgui_key = to_imgui_key(key); orion::to_underlying(imgui_key) != -1) {
            ImGui::GetIO().AddKeyEvent(imgui_key, down);
        }
    }

    void imgui_on_key_press(const orion::events::KeyPress& key_press)
    {
        imgui_on_key(key_press.key, true);
    }

    void imgui_on_key_repeat(const orion::events::KeyRepeat& key_repeat)
    {
        imgui_on_key(key_repeat.key, true);
    }

    void imgui_on_key_release(const orion::events::KeyRelease& key_release)
    {
        imgui_on_key(key_release.key, false);
    }

    void imgui_on_mouse_move(const orion::events::MouseMove& mouse_move)
    {
        const auto position = vector_cast<float>(mouse_move.position);
        ImGui::GetIO().AddMousePosEvent(position.x(), position.y());
    }

    void imgui_on_mouse_button(orion::MouseButton button, bool down)
    {
        // Don't pass X buttons to imgui since it doesn't support them
        if (const auto imgui_button = to_imgui_mouse_button(button); imgui_button != -1) {
            ImGui::GetIO().AddMouseButtonEvent(imgui_button, down);
        }
    }

    void imgui_on_mouse_button_down(const orion::events::MouseButtonDown& mouse_button_down)
    {
        imgui_on_mouse_button(mouse_button_down.button, true);
    }

    void imgui_on_mouse_button_up(const orion::events::MouseButtonUp& mouse_button_up)
    {
        imgui_on_mouse_button(mouse_button_up.button, false);
    }

    void imgui_on_mouse_scoll(const orion::events::MouseScroll& scroll)
    {
        ImGui::GetIO().AddMouseWheelEvent(0.f, static_cast<float>(scroll.delta));
    }

    struct ImGuiPlatformData {
        orion::Window* window;
        orion::handler_index resize_handler;
        orion::handler_index key_press_handler;
        orion::handler_index key_repeat_handler;
        orion::handler_index key_release_handler;
        orion::handler_index mouse_move_handler;
        orion::handler_index mouse_button_down_handler;
        orion::handler_index mouse_button_up_handler;
        orion::handler_index mouse_scroll_handler;
        orion::Clock::time_point last_frame = orion::Clock::now();
    };

    ImGuiPlatformData* imgui_get_platform_data()
    {
        return ImGui::GetCurrentContext() ? static_cast<ImGuiPlatformData*>(ImGui::GetIO().BackendPlatformUserData) : nullptr;
    }

    void imgui_init_platform(const ImGui_ImplOrion_InitDesc& init_desc)
    {
        auto& io = ImGui::GetIO();
        io.BackendPlatformName = "Orion";

        // Set display size
        auto* window = init_desc.window;
        io.DisplaySize = window->size();

        // Create platform data
        auto* platform_data = IM_NEW(ImGuiPlatformData)();
        io.BackendPlatformUserData = platform_data;
        platform_data->window = window;
        platform_data->resize_handler = window->on_resize_end().subscribe(imgui_on_resize);
        auto& keyboard = window->keyboard();
        platform_data->key_press_handler = keyboard.on_key_press().subscribe(imgui_on_key_press);
        platform_data->key_repeat_handler = keyboard.on_key_repeat().subscribe(imgui_on_key_repeat);
        platform_data->key_release_handler = keyboard.on_key_release().subscribe(imgui_on_key_release);
        auto& mouse = window->mouse();
        platform_data->mouse_move_handler = mouse.on_move().subscribe(imgui_on_mouse_move);
        platform_data->mouse_button_down_handler = mouse.on_button_down().subscribe(imgui_on_mouse_button_down);
        platform_data->mouse_button_up_handler = mouse.on_button_up().subscribe(imgui_on_mouse_button_up);
        platform_data->mouse_scroll_handler = mouse.on_scroll().subscribe(imgui_on_mouse_scoll);

        SPDLOG_LOGGER_TRACE(logger(), "ImGui_ImplOrion_Platform initialized");
    }

    void imgui_shutdown_platform()
    {
        auto* platform_data = imgui_get_platform_data();
        if (!platform_data) {
            return;
        }

        // Unregister event callbacks
        auto* window = platform_data->window;
        window->on_resize_end().unsubscribe(platform_data->resize_handler);
        auto& keyboard = window->keyboard();
        keyboard.on_key_press().unsubscribe(platform_data->key_press_handler);
        keyboard.on_key_repeat().unsubscribe(platform_data->key_repeat_handler);
        keyboard.on_key_release().unsubscribe(platform_data->key_release_handler);
        auto& mouse = window->mouse();
        mouse.on_move().unsubscribe(platform_data->mouse_move_handler);
        mouse.on_button_down().unsubscribe(platform_data->mouse_button_down_handler);
        mouse.on_button_up().unsubscribe(platform_data->mouse_button_up_handler);
        mouse.on_scroll().unsubscribe(platform_data->mouse_scroll_handler);

        // Destroy platform data
        IM_DELETE(platform_data);

        SPDLOG_LOGGER_TRACE(logger(), "ImGui_ImplOrion_Platform shut down");
    }

    struct ImGuiFrameData {
        orion::MappedGPUBuffer vertex_buffer;
        orion::MappedGPUBuffer index_buffer;
        ImGuiCSceneBuffer scene_buffer;
    };

    struct ImGuiRendererData {
        orion::RenderDevice* device;
        orion::UniquePipelineLayout pipeline_layout;
        orion::UniquePipeline pipeline;

        std::vector<ImGuiFrameData> frame_data;
        orion::frame_index_t frame_index;

        auto& current_frame() { return frame_data[frame_index]; }

        orion::UniqueImage font_image;
        orion::UniqueImageView font_image_view;
        orion::UniqueSampler font_sampler;

        orion::UniqueDescriptorLayout descriptor_layout;
        orion::UniqueDescriptor descriptor;
    };

    ImGuiRendererData* imgui_get_renderer_data()
    {
        return ImGui::GetCurrentContext() ? static_cast<ImGuiRendererData*>(ImGui::GetIO().BackendRendererUserData) : nullptr;
    }

    orion::UniqueDescriptorLayout imgui_create_descriptor_layout(orion::RenderDevice* device)
    {
        auto descriptor_layout = device->create_descriptor_layout({
            .bindings = {
                {
                    orion::DescriptorBindingDesc{
                        .type = orion::BindingType::SampledImage,
                        .shader_stages = orion::ShaderStageFlags::Pixel,
                        .count = 1,
                    },
                    orion::DescriptorBindingDesc{
                        .type = orion::BindingType::Sampler,
                        .shader_stages = orion::ShaderStageFlags::Pixel,
                        .count = 1,
                    },
                },
            },
        });
        return device->to_unique(descriptor_layout);
    }

    orion::UniquePipelineLayout imgui_create_pipeline_layout(orion::RenderDevice* device, orion::DescriptorLayoutHandle descriptor_layout)
    {
        const auto descriptors = std::array{descriptor_layout};
        const auto push_constants = std::array{
            orion::PushConstantDesc{
                .size = sizeof(ImGuiCSceneBuffer),
                .shader_stages = orion::ShaderStageFlags::Vertex,
            },
        };
        return device->to_unique(device->create_pipeline_layout({
            .descriptors = descriptors,
            .push_constants = push_constants,
        }));
    }

    orion::UniquePipeline imgui_create_pipeline(
        orion::RenderDevice* device,
        orion::ShaderManager* shader_manager,
        orion::PipelineLayoutHandle pipeline_layout,
        orion::RenderPassHandle render_pass)
    {
        // Create shaders
        const auto shader_effect = shader_manager->load_shader_effect("imgui");

        // Create vertex attributes and bindings
        const auto vertex_attributes = std::array{
            orion::VertexAttributeDesc{.name = "POSITION", .format = orion::Format::R32G32_Float},
            orion::VertexAttributeDesc{.name = "TEXCOORD", .format = orion::Format::R32G32_Float},
            orion::VertexAttributeDesc{.name = "COLOR", .format = orion::Format::R8G8B8A8_Unorm},
        };
        const auto vertex_bindings = std::array{
            orion::VertexBinding{vertex_attributes, orion::InputRate::Vertex},
        };

        // Set input assembly description
        const auto input_assembly = orion::InputAssemblyDesc{
            .topology = orion::PrimitiveTopology::TriangleList,
        };

        // Set rasterization description
        const auto rasterization = orion::RasterizationDesc{
            .fill_mode = orion::FillMode::Solid,
            .cull_mode = orion::CullMode::None,
            .front_face = orion::FrontFace::CounterClockWise,
        };

        // Set color blend
        const auto blend_attachments = std::array{
            orion::BlendAttachmentDesc{
                .enable_blend = true,
                .src_blend = orion::BlendFactor::SrcAlpha,
                .dst_blend = orion::BlendFactor::InvertedSrcAlpha,
                .blend_op = orion::BlendOp::Add,
                .src_blend_alpha = orion::BlendFactor::One,
                .dst_blend_alpha = orion::BlendFactor::InvertedSrcAlpha,
                .blend_op_alpha = orion::BlendOp::Add,
                .color_component_flags = orion::ColorComponentFlags::All,
            },
        };
        const auto color_blend = orion::ColorBlendDesc{
            .enable_logic_op = false,
            .logic_op = orion::LogicOp::NoOp,
            .attachments = blend_attachments,
            .blend_constants = {0.f, 0.f, 0.f, 0.f},
        };

        // Set pipeline description
        const auto desc = orion::GraphicsPipelineDesc{
            .shaders = shader_effect.shader_stages(),
            .vertex_bindings = vertex_bindings,
            .pipeline_layout = pipeline_layout,
            .input_assembly = input_assembly,
            .rasterization = rasterization,
            .color_blend = color_blend,
            .render_pass = render_pass,
        };
        return device->make_unique(orion::PipelineHandle_tag{}, desc);
    }

    inline constexpr auto font_image_format = orion::Format::B8G8R8A8_Srgb;

    orion::UniqueImage imgui_create_font_image(
        orion::RenderDevice* device,
        int width,
        int height)
    {
        const auto desc = orion::ImageDesc{
            .type = orion::ImageType::Image2D,
            .format = font_image_format,
            .size = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1},
            .tiling = orion::ImageTiling::Optimal,
            .usage = orion::ImageUsageFlags::SampledImage | orion::ImageUsageFlags::TransferDst,
        };
        return device->make_unique(orion::ImageHandle_tag{}, desc);
    }

    orion::UniqueImageView imgui_create_font_image_view(
        orion::RenderDevice* device,
        orion::ImageHandle font_image)
    {
        const auto desc = orion::ImageViewDesc{
            .image = font_image,
            .type = orion::ImageViewType::View2D,
            .format = font_image_format,
        };
        return device->make_unique(orion::ImageViewHandle_tag{}, desc);
    }

    void imgui_upload_font_image(
        orion::RenderDevice* device,
        std::size_t upload_size,
        const void* pixels,
        orion::CommandAllocator* command_allocator,
        orion::ImageHandle font_image,
        int width,
        int height)
    {
        orion::GPUBufferHandle upload_buffer;
        {
            const auto desc = orion::GPUBufferDesc{
                .size = upload_size,
                .usage = orion::GPUBufferUsageFlags::TransferSrc,
                .host_visible = true,
            };
            upload_buffer = device->create_buffer(desc);
        }

        // Upload image data to buffer
        {
            void* buffer_ptr = device->map(upload_buffer);
            std::memcpy(buffer_ptr, pixels, upload_size);
            device->unmap(upload_buffer);
        }

        // Upload buffer to image
        auto cmd_list = command_allocator->create_command_list();
        cmd_list->begin();
        {
            cmd_list->transition_barrier({.image = font_image, .old_layout = orion::ImageLayout::Undefined, .new_layout = orion::ImageLayout::TransferDst});
            cmd_list->copy_buffer_to_image({
                .src_buffer = upload_buffer,
                .dst_image = font_image,
                .dst_layout = orion::ImageLayout::TransferDst,
                .buffer_offset = 0,
                .image_offset = 0,
                .dst_size = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1},
            });
            cmd_list->transition_barrier({.image = font_image, .old_layout = orion::ImageLayout::TransferDst, .new_layout = orion::ImageLayout::ShaderReadOnly});
        }
        cmd_list->end();
        // Submit the image upload
        device->submit_immediate({
            .queue_type = orion::CommandQueueType::Graphics,
            .command_lists = {{cmd_list.get()}},
        });

        // Destroy upload buffer
        device->destroy(upload_buffer);
    }

    orion::UniqueSampler imgui_create_font_sampler(orion::RenderDevice* device)
    {
        const auto desc = orion::SamplerDesc{
            .filter = orion::Filter::Nearest,
            .address_mode_u = orion::AddressMode::Repeat,
            .address_mode_v = orion::AddressMode::Repeat,
            .address_mode_w = orion::AddressMode::Repeat,
            .mip_load_bias = 0.f,
            .max_anisotropy = 1.f,
            .min_lod = -1000,
            .max_lod = 1000,
        };
        return device->make_unique(orion::SamplerHandle_tag{}, desc);
    }

    orion::UniqueDescriptor imgui_create_font_descriptor(
        orion::RenderDevice* device,
        orion::DescriptorLayoutHandle descriptor_layout,
        orion::ImageViewHandle font_image_view,
        orion::SamplerHandle font_sampler)
    {
        // Create descriptor
        const auto descriptor = device->create_descriptor(descriptor_layout);

        // Update descriptor
        const auto descriptor_bindings = std::array{
            orion::DescriptorBinding{
                .binding = 0,
                .binding_type = orion::BindingType::SampledImage,
                .binding_value = orion::ImageBindingDesc{
                    .image_view_handle = font_image_view,
                    .image_layout = orion::ImageLayout::ShaderReadOnly,
                },
            },
            orion::DescriptorBinding{
                .binding = 1,
                .binding_type = orion::BindingType::Sampler,
                .binding_value = orion::ImageBindingDesc{
                    .sampler_handle = font_sampler,
                },
            },
        };
        device->write_descriptor(descriptor, descriptor_bindings);

        return device->to_unique(descriptor);
    }

    void imgui_init_renderer(const ImGui_ImplOrion_InitDesc& init_desc)
    {
        // Get IO
        auto& io = ImGui::GetIO();
        io.BackendRendererName = "Orion Renderer";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

        // Create renderer data
        auto* renderer_data = IM_NEW(ImGuiRendererData)();
        io.BackendRendererUserData = renderer_data;

        // Get render device
        auto* device = init_desc.device;
        renderer_data->device = device;

        // Get command allocator
        auto* command_allocator = init_desc.command_allocator;

        // Create frames in flight
        renderer_data->frame_data.reserve(orion::frames_in_flight);
        for (int i = 0; i < orion::frames_in_flight; ++i) {
            renderer_data->frame_data.emplace_back(
                orion::MappedGPUBuffer{device, orion::GPUBufferUsageFlags::VertexBuffer},
                orion::MappedGPUBuffer{device, orion::GPUBufferUsageFlags::IndexBuffer});
        }

        // Get the shader manager
        auto* shader_manager = init_desc.shader_manager;

        renderer_data->descriptor_layout = imgui_create_descriptor_layout(device);
        renderer_data->pipeline_layout = imgui_create_pipeline_layout(device, renderer_data->descriptor_layout.get());
        renderer_data->pipeline = imgui_create_pipeline(device, shader_manager, renderer_data->pipeline_layout.get(), init_desc.render_pass);

        // Get font data
        unsigned char* pixels;
        int width;
        int height;
        int bytes_per_pixel;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);
        const auto upload_size = static_cast<std::size_t>(width * height * bytes_per_pixel);

        renderer_data->font_image = imgui_create_font_image(device, width, height);
        renderer_data->font_image_view = imgui_create_font_image_view(device, renderer_data->font_image.get());
        imgui_upload_font_image(device, upload_size, pixels, command_allocator, renderer_data->font_image.get(), width, height);

        renderer_data->font_sampler = imgui_create_font_sampler(device);

        renderer_data->descriptor = imgui_create_font_descriptor(
            device,
            renderer_data->descriptor_layout.get(),
            renderer_data->font_image_view.get(),
            renderer_data->font_sampler.get());

        SPDLOG_LOGGER_TRACE(logger(), "ImGui_ImplOrion_Renderer initialized");
    }

    void imgui_shutdown_renderer()
    {
        // Get renderer data
        auto* renderer_data = imgui_get_renderer_data();
        if (!renderer_data) {
            return;
        }

        // Delete renderer data
        IM_DELETE(renderer_data);

        SPDLOG_LOGGER_TRACE(logger(), "ImGui_ImplOrion_Renderer shut down");
    }
} // namespace

void ImGui_ImplOrion_Init(const ImGui_ImplOrion_InitDesc& desc)
{
    imgui_init_platform(desc);
    imgui_init_renderer(desc);
    SPDLOG_LOGGER_DEBUG(logger(), "ImGui_ImplOrion initialized");
}

void ImGui_ImplOrion_Shutdow()
{
    imgui_shutdown_renderer();
    imgui_shutdown_platform();
    SPDLOG_LOGGER_DEBUG(logger(), "ImGui_ImplOrion shut down");
}

void ImGui_ImplOrion_NewFrame(orion::frame_index_t frame_index)
{
    auto* platform_data = imgui_get_platform_data();
    ORION_ASSERT(platform_data != nullptr && "Did you call ImGui_ImplOrion_Init()?");

    const auto now = orion::Clock::now();
    const std::chrono::duration<float> delta_time = now - platform_data->last_frame;
    platform_data->last_frame = now;
    auto& io = ImGui::GetIO();
    io.DeltaTime = delta_time.count();

    auto* renderer_data = imgui_get_renderer_data();
    renderer_data->frame_index = frame_index;
}

void ImGui_ImplOrion_RenderDrawData(ImDrawData* draw_data, orion::CommandList* cmd_list)
{
    // Don't render if minimized
    if (draw_data->DisplaySize.x <= 0.f || draw_data->DisplaySize.y <= 0.f) {
        return;
    }

    // Don't render if nothing to render
    if (draw_data->TotalVtxCount <= 0) {
        return;
    }

    auto* renderer_data = imgui_get_renderer_data();

    // Get frame
    auto& frame = renderer_data->current_frame();

    // Calculate buffer sizes
    const auto vb_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
    const auto ib_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

    // Resize buffers if needed
    if (vb_size > frame.vertex_buffer.size()) {
        frame.vertex_buffer.resize(orion::max(vb_size, frame.vertex_buffer.size() * 2));
    }
    if (ib_size > frame.index_buffer.size()) {
        frame.index_buffer.resize(orion::max(ib_size, frame.index_buffer.size() * 2));
    }

    // Upload vertex and index data
    auto* vert_ptr = static_cast<ImDrawVert*>(frame.vertex_buffer.ptr());
    auto* idx_ptr = static_cast<ImDrawIdx*>(frame.index_buffer.ptr());
    for (int i = 0; i < draw_data->CmdListsCount; ++i) {
        auto* imgui_cmd_list = draw_data->CmdLists[i];
        std::memcpy(vert_ptr, imgui_cmd_list->VtxBuffer.Data, imgui_cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        std::memcpy(idx_ptr, imgui_cmd_list->IdxBuffer.Data, imgui_cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vert_ptr += imgui_cmd_list->VtxBuffer.Size;
        idx_ptr += imgui_cmd_list->IdxBuffer.Size;
    }

    // Bind pipeline
    cmd_list->bind_pipeline({.pipeline = renderer_data->pipeline.get(), .bind_point = orion::PipelineBindPoint::Graphics});

    // Bind vertex and index buffers
    cmd_list->bind_vertex_buffer({.vertex_buffer = frame.vertex_buffer.handle(), .offset = 0});
    cmd_list->bind_index_buffer({.index_buffer = frame.index_buffer.handle(), .offset = 0, .index_type = orion::IndexType::Uint16});

    // Create orthogonal projection matrix
    const float left = draw_data->DisplayPos.x;
    const float right = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    const float top = draw_data->DisplayPos.y;
    const float bottom = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    frame.scene_buffer.projection = orion::orthographic_rh(left, right, bottom, top, 0.f, 1.f);

    // Push the projection matrix as push constant
    cmd_list->push_constants({
        .pipeline_layout = renderer_data->pipeline_layout.get(),
        .shader_stages = orion::ShaderStageFlags::Vertex,
        .offset = 0,
        .size = sizeof(frame.scene_buffer),
        .values = &frame.scene_buffer,
    });

    // Render command lists
    std::uint32_t global_idx_offset = 0u;
    std::uint32_t global_vtx_offset = 0u;
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int i = 0; i < draw_data->CmdListsCount; ++i) {
        const auto* imgui_cmd_list = draw_data->CmdLists[i];
        for (const auto& draw_cmd : imgui_cmd_list->CmdBuffer) {
            // Project scissor/clipping rectangles into framebuffer space
            ImVec2 clip_min(draw_cmd.ClipRect.x - clip_off.x, draw_cmd.ClipRect.y - clip_off.y);
            ImVec2 clip_max(draw_cmd.ClipRect.z - clip_off.x, draw_cmd.ClipRect.w - clip_off.y);
            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                continue;

            // Set viewport
            cmd_list->set_viewports({
                .position = {},
                .size = {draw_data->DisplaySize.x, draw_data->DisplaySize.y},
                .depth = {0.f, 1.f},
            });

            // Set scissor
            cmd_list->set_scissors({
                .offset = {static_cast<int>(clip_min.x), static_cast<int>(clip_min.y)},
                .size = {static_cast<uint32_t>(clip_max.x - clip_min.x), static_cast<uint32_t>(clip_max.y - clip_min.y)},
            });

            cmd_list->bind_descriptor({
                .bind_point = orion::PipelineBindPoint::Graphics,
                .pipeline_layout = renderer_data->pipeline_layout.get(),
                .index = 0,
                .descriptor = renderer_data->descriptor.get(),
            });

            cmd_list->draw_indexed({
                .index_count = draw_cmd.ElemCount,
                .instance_count = 1,
                .first_index = global_idx_offset + draw_cmd.IdxOffset,
                .vertex_offset = static_cast<std::int32_t>(global_vtx_offset + draw_cmd.VtxOffset),
                .first_instance = 0,
            });
        }
        global_vtx_offset += imgui_cmd_list->VtxBuffer.Size;
        global_idx_offset += imgui_cmd_list->IdxBuffer.Size;
    }
}
