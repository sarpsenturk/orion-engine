#include "imgui_impl_orion.h"

#ifndef ORION_IMGUI_LOG_LEVEL
    #define ORION_IMGUI_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

#include <array>

#include "orion-math/matrix/matrix4.h"
#include "orion-math/matrix/projection.h"

#include "orion-core/clock.h"

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
        orion::clock::time_point last_frame = orion::clock::now();
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

    struct ImGuiRendererData {
        orion::RenderDevice* device;
        orion::ShaderManager* shader_manager;
        orion::UniquePipeline pipeline;

        orion::UniqueGPUBuffer vertex_buffer;
        std::size_t vertex_buffer_size;
        void* vertex_buffer_ptr;

        orion::UniqueGPUBuffer index_buffer;
        std::size_t index_buffer_size;
        void* index_buffer_ptr;

        ImGuiCSceneBuffer scene_buffer;

        orion::UniqueImage font_image;
        orion::UniqueImageView font_image_view;
        orion::UniqueSampler font_sampler;
    };

    ImGuiRendererData* imgui_get_renderer_data()
    {
        return ImGui::GetCurrentContext() ? static_cast<ImGuiRendererData*>(ImGui::GetIO().BackendRendererUserData) : nullptr;
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

        // Get the shader manager
        auto* shader_manager = init_desc.shader_manager;
        renderer_data->shader_manager = shader_manager;

        // Create pipeline
        {
            // Create shaders
            const auto* vertex_shader = shader_manager->load("imgui.vs").second;
            const auto* pixel_shader = shader_manager->load("imgui.ps").second;
            const auto shader_effect = orion::ShaderEffect{std::array{vertex_shader, pixel_shader}};

            // Create vertex attributes and bindings
            const auto vertex_attributes = std::array{
                orion::VertexAttributeDesc{.name = "POSITION", .format = orion::Format::R32G32_Float},
                orion::VertexAttributeDesc{.name = "TEXCOORD", .format = orion::Format::R32G32_Float},
                orion::VertexAttributeDesc{.name = "COLOR", .format = orion::Format::R8G8B8A8_Unorm},
            };
            const auto vertex_bindings = std::array{
                orion::VertexBinding{vertex_attributes, orion::InputRate::Vertex},
            };

            // TODO: Create pipeline layout
            const auto push_constants = std::array{
                orion::PushConstantDesc{
                    .size = sizeof(ImGuiCSceneBuffer),
                    .shader_stages = orion::ShaderStageFlags::Vertex,
                },
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

            // Set attachment list
            const auto color_attachments = std::array{
                orion::AttachmentDesc{.format = orion::Format::B8G8R8A8_Srgb},
            };
            const auto attachment_list = orion::AttachmentList{color_attachments};

            // Set pipeline description
            const auto desc = orion::GraphicsPipelineDesc{
                {},
                vertex_bindings,
                {},
                input_assembly,
                rasterization,
                color_blend,
                attachment_list,
            };
            renderer_data->pipeline = device->make_unique(orion::PipelineHandle_tag{}, desc);
        }

        // Get font data
        unsigned char* pixels;
        int width;
        int height;
        int bytes_per_pixel;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);
        const auto upload_size = static_cast<std::size_t>(width * height * bytes_per_pixel);
        const auto font_image_format = orion::Format::R8G8B8A8_Unorm;

        // Create font image
        {
            const auto desc = orion::ImageDesc{
                .type = orion::ImageType::Image2D,
                .format = font_image_format,
                .size = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1},
                .tiling = orion::ImageTiling::Optimal,
                .usage = orion::ImageUsageFlags::SampledImage | orion::ImageUsageFlags::TransferDst,
            };
            renderer_data->font_image = device->make_unique(orion::ImageHandle_tag{}, desc);
        }

        // Create font image view
        {
            const auto desc = orion::ImageViewDesc{
                .image = renderer_data->font_image.get(),
                .type = orion::ImageViewType::View2D,
                .format = font_image_format,
            };
            renderer_data->font_image_view = device->make_unique(orion::ImageViewHandle_tag{}, desc);
        }

        // Create font upload buffer
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

        // TODO: Copy buffer to image

        // Destroy upload buffer
        device->destroy(upload_buffer);

        // Create font sampler
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
            renderer_data->font_sampler = device->make_unique(orion::SamplerHandle_tag{}, desc);
        }

        // TODO: Set font texture as descriptor set handle

        SPDLOG_LOGGER_TRACE(logger(), "ImGui_ImplOrion_Renderer initialized");
    }

    void imgui_shutdown_renderer()
    {
        // Get renderer data
        auto* renderer_data = imgui_get_renderer_data();
        if (!renderer_data) {
            return;
        }

        auto* device = renderer_data->device;

        // Unmap buffers
        if (renderer_data->vertex_buffer_ptr) {
            device->unmap(renderer_data->vertex_buffer.get());
        }
        if (renderer_data->index_buffer_ptr) {
            device->unmap(renderer_data->index_buffer.get());
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

void ImGui_ImplOrion_NewFrame()
{
    auto* platform_data = imgui_get_platform_data();
    ORION_ASSERT(platform_data != nullptr && "Did you call ImGui_ImplOrion_Init()?");

    const auto now = orion::clock::now();
    const std::chrono::duration<float> delta_time = now - platform_data->last_frame;
    platform_data->last_frame = now;
    auto& io = ImGui::GetIO();
    io.DeltaTime = delta_time.count();
}

void ImGui_ImplOrion_RenderDrawData(ImDrawData* draw_data)
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
    auto* device = renderer_data->device;

    // Calculate buffer sizes
    const auto vb_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
    const auto ib_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

    // Resize buffers if needed
    if (renderer_data->vertex_buffer_size < vb_size) {
        if (renderer_data->vertex_buffer) {
            device->unmap(renderer_data->vertex_buffer.get());
        }
        const auto desc = orion::GPUBufferDesc{
            .size = vb_size,
            .usage = orion::GPUBufferUsageFlags::VertexBuffer,
            .host_visible = true,
        };
        renderer_data->vertex_buffer = device->make_unique(orion::GPUBufferHandle_tag{}, desc);
        renderer_data->vertex_buffer_size = vb_size;
        renderer_data->vertex_buffer_ptr = device->map(renderer_data->vertex_buffer.get());
    }
    if (renderer_data->index_buffer_size < ib_size) {
        if (renderer_data->index_buffer) {
            device->unmap(renderer_data->index_buffer.get());
        }
        const auto desc = orion::GPUBufferDesc{
            .size = ib_size,
            .usage = orion::GPUBufferUsageFlags::IndexBuffer,
            .host_visible = true,
        };
        renderer_data->index_buffer = device->make_unique(orion::GPUBufferHandle_tag{}, desc);
        renderer_data->index_buffer_size = ib_size;
        renderer_data->index_buffer_ptr = device->map(renderer_data->index_buffer.get());
    }

    // Upload vertex and index data
    auto* vert_ptr = static_cast<ImDrawVert*>(renderer_data->vertex_buffer_ptr);
    auto* idx_ptr = static_cast<ImDrawIdx*>(renderer_data->index_buffer_ptr);
    for (int i = 0; i < draw_data->CmdListsCount; ++i) {
        auto* cmd_list = draw_data->CmdLists[i];
        std::memcpy(vert_ptr, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        std::memcpy(idx_ptr, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vert_ptr += cmd_list->VtxBuffer.Size;
        idx_ptr += cmd_list->IdxBuffer.Size;
    }

    // Create orthogonal projection matrix
    const float left = draw_data->DisplayPos.x;
    const float right = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    const float top = draw_data->DisplayPos.y;
    const float bottom = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    renderer_data->scene_buffer.projection = orion::orthographic_rh(left, right, bottom, top, 0.f, 1.f);

    // TODO: Push the projection matrix

    // Render command lists
    std::uint32_t global_idx_offset = 0u;
    std::uint32_t global_vtx_offset = 0u;
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int i = 0; i < draw_data->CmdListsCount; ++i) {
        const auto* cmd_list = draw_data->CmdLists[i];
        for (const auto& draw_cmd : cmd_list->CmdBuffer) {
            // Project scissor/clipping rectangles into framebuffer space
            ImVec2 clip_min(draw_cmd.ClipRect.x - clip_off.x, draw_cmd.ClipRect.y - clip_off.y);
            ImVec2 clip_max(draw_cmd.ClipRect.z - clip_off.x, draw_cmd.ClipRect.w - clip_off.y);
            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                continue;
            // Set viewport
            const auto viewport = orion::Viewport{
                .position = {},
                .size = {draw_data->DisplaySize.x, draw_data->DisplaySize.y},
                .depth = {0.f, 1.f},
            };
            // Set scissor
            const auto scissor = orion::Scissor{
                .offset = {static_cast<int>(clip_min.x), static_cast<int>(clip_min.y)},
                .size = {static_cast<uint32_t>(clip_max.x - clip_min.x), static_cast<uint32_t>(clip_max.y - clip_min.y)},
            };

            // TODO: Bind descriptor set

            // TODO: Draw indexed
        }
        global_vtx_offset += cmd_list->VtxBuffer.Size;
        global_idx_offset += cmd_list->IdxBuffer.Size;
    }
}
