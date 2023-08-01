#include "imgui_impl_orion.h"

#ifndef ORION_IMGUI_LOG_LEVEL
    #define ORION_IMGUI_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

#include "orion-renderer/shader_compiler.h"

#include <array>

#include "orion-math/matrix/matrix4.h"
#include "orion-math/matrix/transformation.h"

namespace
{
    constexpr auto imgui_shader_src = R"hlsl(
struct VsInput {
    float2 position : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct VsOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct SceneConstants {
    row_major float4x4 projection;
};

[[vk::push_constant]]
SceneConstants scene_data;

VsOutput vs_main(VsInput input)
{
    VsOutput output;
    output.position = mul(float4(input.position, -.5f, 1.f), scene_data.projection);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}

struct FsInput {
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

float4 fs_main(FsInput input) : SV_Target
{
    return input.color;
}
)hlsl";

    struct ImGuiCSceneBuffer {
        orion::Matrix4_f projection;
    };

    auto* logger()
    {
        static const auto logger = orion::create_logger("orion-imgui", static_cast<spdlog::level::level_enum>(ORION_IMGUI_LOG_LEVEL));
        return logger.get();
    }

    ImGuiKey to_imgui_key(orion::KeyCode key_code)
    {
        return {};
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
        ImGui::GetIO().AddKeyEvent(to_imgui_key(key), down);
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
        orion::UniquePipeline pipeline;

        orion::UniqueGPUBuffer vertex_buffer;
        std::size_t vertex_buffer_size;
        void* vertex_buffer_ptr;

        orion::UniqueGPUBuffer index_buffer;
        std::size_t index_buffer_size;
        void* index_buffer_ptr;

        ImGuiCSceneBuffer scene_buffer;
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

        // Create pipeline
        {
            auto shader_compiler = orion::ShaderCompiler{};

            // Create shaders
            auto vs_module = [device, &shader_compiler]() {
                auto compile_result = shader_compiler.compile({
                    .source_string = imgui_shader_src,
                    .entry_point = "vs_main",
                    .shader_type = orion::ShaderStage::Vertex,
                    .object_type = device->shader_object_type(),
                });
                const auto desc = orion::ShaderModuleDesc{.byte_code = compile_result.binary};
                return device->make_unique(orion::ShaderModuleHandle_tag{}, desc);
            }();
            auto fs_module = [device, &shader_compiler]() {
                auto compile_result = shader_compiler.compile({
                    .source_string = imgui_shader_src,
                    .entry_point = "fs_main",
                    .shader_type = orion::ShaderStage::Fragment,
                    .object_type = device->shader_object_type(),
                });
                const auto desc = orion::ShaderModuleDesc{.byte_code = compile_result.binary};
                return device->make_unique(orion::ShaderModuleHandle_tag{}, desc);
            }();
            const auto shaders = std::array{
                orion::ShaderStageDesc{
                    .module = vs_module.get(),
                    .stage = orion::ShaderStage::Vertex,
                    .entry_point = "vs_main",
                },
                orion::ShaderStageDesc{
                    .module = fs_module.get(),
                    .stage = orion::ShaderStage::Fragment,
                    .entry_point = "fs_main",
                },
            };

            // Create vertex attributes and bindings
            const auto vertex_attributes = std::array{
                orion::VertexAttributeDesc{.name = "POSITION", .format = orion::Format::R32G32_Float},
                orion::VertexAttributeDesc{.name = "TEXCOORD", .format = orion::Format::R32G32_Float},
                orion::VertexAttributeDesc{.name = "COLOR", .format = orion::Format::R8G8B8A8_Unorm},
            };
            const auto vertex_bindings = std::array{
                orion::VertexBinding{vertex_attributes, orion::InputRate::Vertex},
            };
            const auto push_constants = std::array{
                orion::PushConstantDesc{
                    .size = sizeof(ImGuiCSceneBuffer),
                    .shader_stages = orion::ShaderStage::Vertex,
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

            // Set attachment list
            const auto color_attachments = std::array{
                orion::AttachmentDesc{.format = orion::Format::B8G8R8A8_Srgb},
            };
            const auto attachment_list = orion::AttachmentList{color_attachments};

            // Set pipeline description
            const auto desc = orion::GraphicsPipelineDesc{
                shaders,
                vertex_bindings,
                {},
                push_constants,
                input_assembly,
                rasterization,
                attachment_list,
            };
            renderer_data->pipeline = device->make_unique(orion::PipelineHandle_tag{}, desc);
        }

        // Fake load font atlas
        // TODO: Actually load and build font atlas
        {
            unsigned char* font_atlas;
            int width;
            int height;
            io.Fonts->GetTexDataAsAlpha8(&font_atlas, &width, &height);
        }

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
}

void ImGui_ImplOrion_RenderDrawData(ImDrawData* draw_data, orion::CommandList& command_list)
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
            .usage = orion::GPUBufferUsage::VertexBuffer,
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
            .usage = orion::GPUBufferUsage::IndexBuffer,
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

    // Push the projection matrix
    {
        auto* cmd_push_constants = command_list.add_command<orion::CmdPushConstants>({});
        cmd_push_constants->pipeline = renderer_data->pipeline.get();
        cmd_push_constants->shader_stages = orion::ShaderStage::Vertex;
        cmd_push_constants->offset = 0;
        cmd_push_constants->size = sizeof(ImGuiCSceneBuffer);
        cmd_push_constants->data = &renderer_data->scene_buffer;
    }

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
                .size = {static_cast<uint32_t>(clip_max.x), static_cast<uint32_t>(clip_max.y)},
            };

            auto* cmd_draw_indexed = command_list.add_command<orion::CmdDrawIndexed>({});
            cmd_draw_indexed->vertex_buffer = renderer_data->vertex_buffer.get();
            cmd_draw_indexed->index_buffer = renderer_data->index_buffer.get();
            cmd_draw_indexed->index_type = orion::IndexType::Uint16;
            cmd_draw_indexed->graphics_pipeline = renderer_data->pipeline.get();
            cmd_draw_indexed->viewport = viewport;
            cmd_draw_indexed->scissor = scissor;
            cmd_draw_indexed->vertex_offset = global_vtx_offset + draw_cmd.VtxOffset;
            cmd_draw_indexed->index_offset = global_idx_offset + draw_cmd.IdxOffset;
            cmd_draw_indexed->index_count = draw_cmd.ElemCount;
        }
        global_vtx_offset += cmd_list->VtxBuffer.Size;
        global_idx_offset += cmd_list->IdxBuffer.Size;
    }
}
