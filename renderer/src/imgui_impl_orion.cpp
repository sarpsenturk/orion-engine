#include "imgui_impl_orion.h"

#include "orion-assets/config.h"

#include "orion-renderer/config.h"
#include "orion-renderer/render_context.h"

#include "orion-renderapi/device_resource.h"
#include "orion-renderapi/render_device.h"

#include "orion-utils/assertion.h"
#include "orion-utils/static_vector.h"

#include "orion-core/filesystem.h"

#include "orion-math/vector/vector2.h"

#include <array>
#include <cstring>
#include <span>

using namespace orion;

namespace
{
    struct ImGui_ImplOrion_Buffers {
        UniqueGPUBuffer vertex_buffer;
        std::size_t vertex_buffer_size = 0;
        UniqueGPUBuffer index_buffer;
        std::size_t index_buffer_size = 0;
    };

    struct ImGui_ImplOrion_Data {
        UniqueImage font_image;
        UniqueImageView font_image_view;
        UniqueSampler font_sampler;
        UniqueDescriptorLayout font_descriptor_layout;
        UniqueDescriptorPool descriptor_pool;
        UniqueDescriptor font_descriptor;

        UniquePipelineLayout pipeline_layout;
        UniqueRenderPass render_pass;
        UniquePipeline pipeline;

        static_vector<ImGui_ImplOrion_Buffers, frames_in_flight> buffers;
        RenderContext* render_context;

        auto& get_buffers() { return buffers[render_context->current_frame_index()]; }
    };

    struct ImGui_ImplOrion_Constants {
        Vector2_f scale;
        Vector2_f translation;
    };

    ImGui_ImplOrion_Data* ImGui_ImplOrion_GetBackendData()
    {
        return ImGui::GetCurrentContext() ? static_cast<ImGui_ImplOrion_Data*>(ImGui::GetIO().BackendRendererUserData) : nullptr;
    }

    UniquePipelineLayout ImGui_ImplOrion_Create_PipelineLayout(RenderDevice* device, DescriptorLayoutHandle descriptor_layout)
    {
        return device->make_unique<PipelineLayoutHandle_tag>(PipelineLayoutDesc{
            .descriptors = {{descriptor_layout}},
            .push_constants = {{
                PushConstantDesc{
                    .size = sizeof(ImGui_ImplOrion_Constants),
                    .shader_stages = orion::ShaderStageFlags::Vertex},
            }},
        });
    }

    UniqueRenderPass ImGui_ImplOrion_Create_Renderpass(RenderDevice* device)
    {
        return device->make_unique<RenderPassHandle_tag>(RenderPassDesc{
            .color_attachments = {{
                AttachmentDesc{
                    .format = orion::Format::B8G8R8A8_Srgb,
                },
            }},
            .bind_point = orion::PipelineBindPoint::Graphics,
        });
    }

    UniquePipeline ImGui_ImplOrion_Create_Pipeline(RenderDevice* device, PipelineLayoutHandle pipeline_layout, RenderPassHandle render_pass)
    {
        const auto vs_shader = device->make_unique<ShaderModuleHandle_tag>(ShaderModuleDesc{
            .byte_code = binary_input_file(FilePath{device->shader_base_path()} / "imgui.vs").read_all(),
        });
        const auto ps_shader = device->make_unique<ShaderModuleHandle_tag>(ShaderModuleDesc{
            .byte_code = binary_input_file(FilePath{device->shader_base_path()} / "imgui.ps").read_all(),
        });
        const auto shaders = std::array{
            ShaderStageDesc{
                .module = vs_shader.get(),
                .stage = ShaderStageFlags::Vertex,
                .entry_point = ORION_VS_ENTRY,
            },
            ShaderStageDesc{
                .module = ps_shader.get(),
                .stage = ShaderStageFlags::Pixel,
                .entry_point = ORION_PS_ENTRY,
            },
        };

        const auto vertex_attributes = std::array{
            VertexAttributeDesc{.name = "POSITION", .format = orion::Format::R32G32_Float},
            VertexAttributeDesc{.name = "TEXCOORD", .format = orion::Format::R32G32_Float},
            VertexAttributeDesc{.name = "COLOR", .format = orion::Format::R8G8B8A8_Unorm},
        };

        const auto input_assembly = InputAssemblyDesc{
            .topology = orion::PrimitiveTopology::TriangleList,
        };

        const auto rasterization = RasterizationDesc{
            .fill_mode = orion::FillMode::Solid,
            .cull_mode = orion::CullMode::Back,
            .front_face = orion::FrontFace::ClockWise,
        };

        const auto color_blend_attachments = std::array{
            BlendAttachmentDesc{
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
        const auto color_blend = ColorBlendDesc{
            .attachments = color_blend_attachments,
        };

        return device->make_unique<PipelineHandle_tag>(GraphicsPipelineDesc{
            shaders,
            vertex_attributes,
            pipeline_layout,
            input_assembly,
            rasterization,
            color_blend,
            render_pass,
        });
    }

    UniqueImage ImGui_ImplOrion_Create_FontImage(RenderDevice* device, RenderContext* context)
    {
        unsigned char* pixels;
        int width;
        int height;
        ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        const auto image_size = Vector3_u{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1u};
        const auto image = device->create_image({
            .type = orion::ImageType::Image2D,
            .format = orion::Format::R8G8B8A8_Unorm,
            .size = image_size,
            .tiling = orion::ImageTiling::Optimal,
            .usage = orion::ImageUsageFlags::TransferDst | orion::ImageUsageFlags::SampledImage,
            .host_visible = false,
        });

        const auto upload_size = std::size_t{width * height * 4 * sizeof(char)};
        context->copy_image_staging({
            .bytes = std::span{reinterpret_cast<std::byte*>(pixels), upload_size}, // ugly but ok? https://github.com/Microsoft/GSL/issues/589
            .dst = image,
            .dst_initial_layout = orion::ImageLayout::Undefined,
            .dst_final_layout = orion::ImageLayout::ShaderReadOnly,
            .dst_size = image_size,
        });

        return device->to_unique(image);
    }

    UniqueImageView ImGui_ImplOrion_Create_FontImageView(RenderDevice* device, ImageHandle font_image)
    {
        return device->make_unique<ImageViewHandle_tag>(ImageViewDesc{
            .image = font_image,
            .type = orion::ImageViewType::View2D,
            .format = orion::Format::R8G8B8A8_Unorm,
        });
    }

    UniqueSampler ImGui_ImplOrion_Create_FontSampler(RenderDevice* device)
    {
        return device->make_unique<SamplerHandle_tag>(SamplerDesc{
            .filter = orion::Filter::Nearest,
            .address_mode_u = orion::AddressMode::Repeat,
            .address_mode_v = orion::AddressMode::Repeat,
            .address_mode_w = orion::AddressMode::Repeat,
            .mip_lod_bias = 0.f,
            .max_anisotropy = 1.f,
            .min_lod = -1000,
            .max_lod = 1000,
        });
    }

    UniqueDescriptorLayout ImGui_ImplOrion_Create_FontDescriptorLayout(RenderDevice* device)
    {
        return device->make_unique<DescriptorLayoutHandle_tag>(DescriptorLayoutDesc{
            .bindings = {{
                DescriptorBindingDesc{
                    .type = orion::DescriptorType::SampledImage,
                    .shader_stages = orion::ShaderStageFlags::Pixel,
                    .count = 1,
                },
                DescriptorBindingDesc{
                    .type = orion::DescriptorType::Sampler,
                    .shader_stages = orion::ShaderStageFlags::Pixel,
                    .count = 1,
                },
            }},
        });
    }

    UniqueDescriptorPool ImGui_ImplOrion_Create_DescriptorPool(RenderDevice* device)
    {
        return device->make_unique<DescriptorPoolHandle_tag>(DescriptorPoolDesc{
            .max_descriptors = 2,
            .flags = {},
            .sizes = {{
                DescriptorPoolSize{
                    .type = orion::DescriptorType::SampledImage,
                    .count = 1,
                },
                DescriptorPoolSize{
                    .type = orion::DescriptorType::Sampler,
                    .count = 1,
                },
            }},
        });
    }

    UniqueDescriptor ImGui_ImplOrion_Create_FontDescriptor(RenderDevice* device, DescriptorLayoutHandle layout, DescriptorPoolHandle descriptor_pool, ImageViewHandle font_image_view, SamplerHandle sampler)
    {
        const auto descriptor = device->create_descriptor(layout, descriptor_pool);
        const auto image_write = ImageDescriptorDesc{
            .image_view_handle = font_image_view,
            .image_layout = orion::ImageLayout::ShaderReadOnly,
        };
        const auto sampler_write = ImageDescriptorDesc{
            .sampler_handle = sampler,
        };
        const auto writes = std::array{
            DescriptorWrite{
                .binding = 0,
                .descriptor_type = orion::DescriptorType::SampledImage,
                .array_start = 0,
                .images = {&image_write, 1},
            },
            DescriptorWrite{
                .binding = 1,
                .descriptor_type = orion::DescriptorType::Sampler,
                .array_start = 0,
                .images = {&sampler_write, 1},
            },
        };
        device->write_descriptor(descriptor, writes);
        return device->to_unique(descriptor);
    }

    ImGuiKey ImGui_ImplOrion_KeycodeToImGuiKey(KeyCode key)
    {
        switch (key) {
            case KeyCode::Backspace:
                return ImGuiKey_Backspace;
            case KeyCode::Tab:
                return ImGuiKey_Tab;
            case KeyCode::Enter:
                return ImGuiKey_Enter;
            case KeyCode::Escape:
                return ImGuiKey_Escape;
            case KeyCode::Space:
                return ImGuiKey_Space;
            case KeyCode::Quote:
                return ImGuiKey_Apostrophe; // Typically ImGui uses Apostrophe instead of Quote
            case KeyCode::Comma:
                return ImGuiKey_Comma;
            case KeyCode::Minus:
                return ImGuiKey_Minus;
            case KeyCode::Period:
                return ImGuiKey_Period;
            case KeyCode::Slash:
                return ImGuiKey_Slash;
            case KeyCode::Alpha0:
                return ImGuiKey_0;
            case KeyCode::Alpha1:
                return ImGuiKey_1;
            case KeyCode::Alpha2:
                return ImGuiKey_2;
            case KeyCode::Alpha3:
                return ImGuiKey_3;
            case KeyCode::Alpha4:
                return ImGuiKey_4;
            case KeyCode::Alpha5:
                return ImGuiKey_5;
            case KeyCode::Alpha6:
                return ImGuiKey_6;
            case KeyCode::Alpha7:
                return ImGuiKey_7;
            case KeyCode::Alpha8:
                return ImGuiKey_8;
            case KeyCode::Alpha9:
                return ImGuiKey_9;
            case KeyCode::Semicolon:
                return ImGuiKey_Semicolon;
            case KeyCode::Equal:
                return ImGuiKey_Equal;
            case KeyCode::KeyA:
                return ImGuiKey_A;
            case KeyCode::KeyB:
                return ImGuiKey_B;
            case KeyCode::KeyC:
                return ImGuiKey_C;
            case KeyCode::KeyD:
                return ImGuiKey_D;
            case KeyCode::KeyE:
                return ImGuiKey_E;
            case KeyCode::KeyF:
                return ImGuiKey_F;
            case KeyCode::KeyG:
                return ImGuiKey_G;
            case KeyCode::KeyH:
                return ImGuiKey_H;
            case KeyCode::KeyI:
                return ImGuiKey_I;
            case KeyCode::KeyJ:
                return ImGuiKey_J;
            case KeyCode::KeyK:
                return ImGuiKey_K;
            case KeyCode::KeyL:
                return ImGuiKey_L;
            case KeyCode::KeyM:
                return ImGuiKey_M;
            case KeyCode::KeyN:
                return ImGuiKey_N;
            case KeyCode::KeyO:
                return ImGuiKey_O;
            case KeyCode::KeyP:
                return ImGuiKey_P;
            case KeyCode::KeyQ:
                return ImGuiKey_Q;
            case KeyCode::KeyR:
                return ImGuiKey_R;
            case KeyCode::KeyS:
                return ImGuiKey_S;
            case KeyCode::KeyT:
                return ImGuiKey_T;
            case KeyCode::KeyU:
                return ImGuiKey_U;
            case KeyCode::KeyV:
                return ImGuiKey_V;
            case KeyCode::KeyW:
                return ImGuiKey_W;
            case KeyCode::KeyX:
                return ImGuiKey_X;
            case KeyCode::KeyY:
                return ImGuiKey_Y;
            case KeyCode::KeyZ:
                return ImGuiKey_Z;
            case KeyCode::LeftBracket:
                return ImGuiKey_LeftBracket;
            case KeyCode::Backslash:
                return ImGuiKey_Backslash;
            case KeyCode::RightBracket:
                return ImGuiKey_RightBracket;
            case KeyCode::Backtick:
                return ImGuiKey_GraveAccent; // Typically ImGui uses GraveAccent instead of Backtick
            case KeyCode::Delete:
                return ImGuiKey_Delete;
            case KeyCode::Insert:
                return ImGuiKey_Insert;
            case KeyCode::Home:
                return ImGuiKey_Home;
            case KeyCode::End:
                return ImGuiKey_End;
            case KeyCode::PageUp:
                return ImGuiKey_PageUp;
            case KeyCode::PageDown:
                return ImGuiKey_PageDown;
            case KeyCode::PrintScreen:
                return ImGuiKey_PrintScreen;
            case KeyCode::ScrollLock:
                return ImGuiKey_ScrollLock;
            case KeyCode::Pause:
                return ImGuiKey_Pause;
            case KeyCode::F1:
                return ImGuiKey_F1;
            case KeyCode::F2:
                return ImGuiKey_F2;
            case KeyCode::F3:
                return ImGuiKey_F3;
            case KeyCode::F4:
                return ImGuiKey_F4;
            case KeyCode::F5:
                return ImGuiKey_F5;
            case KeyCode::F6:
                return ImGuiKey_F6;
            case KeyCode::F7:
                return ImGuiKey_F7;
            case KeyCode::F8:
                return ImGuiKey_F8;
            case KeyCode::F9:
                return ImGuiKey_F9;
            case KeyCode::F10:
                return ImGuiKey_F10;
            case KeyCode::F11:
                return ImGuiKey_F11;
            case KeyCode::F12:
                return ImGuiKey_F12;
            case KeyCode::Caps:
                return ImGuiKey_CapsLock;
            case KeyCode::Shift:
                return ImGuiKey_ModShift;
            case KeyCode::Control:
                return ImGuiKey_ModCtrl;
            case KeyCode::Alt:
                return ImGuiKey_ModAlt;
            case KeyCode::Command:
                return ImGuiKey_ModSuper;
            case KeyCode::Num0:
                return ImGuiKey_Keypad0;
            case KeyCode::Num1:
                return ImGuiKey_Keypad1;
            case KeyCode::Num2:
                return ImGuiKey_Keypad2;
            case KeyCode::Num3:
                return ImGuiKey_Keypad3;
            case KeyCode::Num4:
                return ImGuiKey_Keypad4;
            case KeyCode::Num5:
                return ImGuiKey_Keypad5;
            case KeyCode::Num6:
                return ImGuiKey_Keypad6;
            case KeyCode::Num7:
                return ImGuiKey_Keypad7;
            case KeyCode::Num8:
                return ImGuiKey_Keypad8;
            case KeyCode::Num9:
                return ImGuiKey_Keypad9;
            case KeyCode::NumDivide:
                return ImGuiKey_KeypadDivide;
            case KeyCode::NumMultiply:
                return ImGuiKey_KeypadMultiply;
            case KeyCode::NumMinus:
                return ImGuiKey_KeypadSubtract;
            case KeyCode::NumPlus:
                return ImGuiKey_KeypadAdd;
            case KeyCode::NumEnter:
                return ImGuiKey_KeypadEnter;
            case KeyCode::NumPeriod:
                return ImGuiKey_KeypadDecimal;
            case KeyCode::NumLock:
                return ImGuiKey_NumLock;
            case KeyCode::LeftArrow:
                return ImGuiKey_LeftArrow;
            case KeyCode::UpArrow:
                return ImGuiKey_UpArrow;
            case KeyCode::RightArrow:
                return ImGuiKey_RightArrow;
            case KeyCode::DownArrow:
                return ImGuiKey_DownArrow;
            default:
                return ImGuiKey_None;
        }
    }

    ImGuiMouseButton ImGui_ImplOrion_MouseButtonToImGuiButton(MouseButton button)
    {
        switch (button) {
            case MouseButton::Unknown:
                break;
            case MouseButton::Left:
                return ImGuiMouseButton_Left;
            case MouseButton::Right:
                return ImGuiMouseButton_Right;
            case MouseButton::Middle:
                return ImGuiMouseButton_Middle;
            case MouseButton::X1:
                break;
            case MouseButton::X2:
                break;
        }
        return -1;
    }
} // namespace

void ImGui_ImplOrion_Init(const ImGui_ImplOrion_Desc& desc)
{
    auto& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();

    ORION_ASSERT(io.BackendPlatformUserData == nullptr && "ImGui backend already initialized");
    ORION_ASSERT(io.BackendRendererUserData == nullptr && "ImGui backend already initialized");

    auto* backend_data = IM_NEW(ImGui_ImplOrion_Data);
    io.BackendRendererUserData = backend_data;
    io.BackendPlatformUserData = backend_data;
    io.BackendRendererName = "imgui_impl_orion";
    io.BackendPlatformName = "imgui_impl_orion";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    backend_data->font_image = ImGui_ImplOrion_Create_FontImage(desc.device, desc.context);
    backend_data->font_image_view = ImGui_ImplOrion_Create_FontImageView(desc.device, backend_data->font_image.get());
    backend_data->font_sampler = ImGui_ImplOrion_Create_FontSampler(desc.device);
    backend_data->font_descriptor_layout = ImGui_ImplOrion_Create_FontDescriptorLayout(desc.device);
    backend_data->descriptor_pool = ImGui_ImplOrion_Create_DescriptorPool(desc.device);
    backend_data->font_descriptor = ImGui_ImplOrion_Create_FontDescriptor(desc.device,
                                                                          backend_data->font_descriptor_layout.get(),
                                                                          backend_data->descriptor_pool.get(),
                                                                          backend_data->font_image_view.get(),
                                                                          backend_data->font_sampler.get());
    backend_data->pipeline_layout = ImGui_ImplOrion_Create_PipelineLayout(desc.device, backend_data->font_descriptor_layout.get());
    backend_data->render_pass = ImGui_ImplOrion_Create_Renderpass(desc.device);
    backend_data->pipeline = ImGui_ImplOrion_Create_Pipeline(desc.device, backend_data->pipeline_layout.get(), backend_data->render_pass.get());
    backend_data->render_context = desc.context;

    io.DisplaySize.x = static_cast<float>(desc.display_size.x());
    io.DisplaySize.y = static_cast<float>(desc.display_size.y());
}

void ImGui_ImplOrion_Shutdown()
{
    auto& io = ImGui::GetIO();
    ORION_ASSERT(io.BackendRendererUserData != nullptr && "ImGui backend not initialized");
    io.BackendRendererName = nullptr;
    IM_FREE(io.BackendRendererUserData);
    io.BackendRendererUserData = nullptr;

    ORION_ASSERT(io.BackendPlatformUserData != nullptr && "ImGui backend not initialized");
    io.BackendPlatformUserData = nullptr;
    io.BackendPlatformName = nullptr;
}

void ImGui_ImplOrion_NewFrame()
{
    ORION_ASSERT(ImGui_ImplOrion_GetBackendData() != nullptr && "ImGui backend not initialized! Did you call ImGui_ImplOrion_Init()?");
    ImGui::GetIO().DeltaTime = 0.016f; // Fixed to 60fps TODO: change this?!
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

    auto* backend = ImGui_ImplOrion_GetBackendData();
    auto& buffers = backend->get_buffers();

    // Resize buffers if needed
    {
        // Calculate buffer sizes
        const auto vb_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
        const auto ib_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

        if (buffers.vertex_buffer_size < vb_size) {
            auto* device = backend->render_context->device();
            buffers.vertex_buffer = device->make_unique<GPUBufferHandle_tag>(GPUBufferDesc{
                .size = vb_size,
                .usage = orion::GPUBufferUsageFlags::VertexBuffer,
                .host_visible = true,
            });
            buffers.vertex_buffer_size = vb_size;
        }
        if (buffers.index_buffer_size < ib_size) {
            auto* device = backend->render_context->device();
            buffers.index_buffer = device->make_unique<GPUBufferHandle_tag>(GPUBufferDesc{
                .size = ib_size,
                .usage = orion::GPUBufferUsageFlags::IndexBuffer,
                .host_visible = true,
            });
            buffers.index_buffer_size = ib_size;
        }
    }

    // Upload vertex and index data
    {
        void* vtx_ptr = backend->render_context->device()->map(buffers.vertex_buffer.get());
        void* idx_ptr = backend->render_context->device()->map(buffers.index_buffer.get());
        for (auto* imgui_cmd_list : std::span{draw_data->CmdLists, static_cast<std::size_t>(draw_data->CmdListsCount)}) {
            const auto vertices = std::as_bytes(std::span{imgui_cmd_list->VtxBuffer});
            std::memcpy(vtx_ptr, vertices.data(), vertices.size_bytes());
            vtx_ptr = static_cast<char*>(vtx_ptr) + vertices.size_bytes();
            const auto indices = std::as_bytes(std::span{imgui_cmd_list->IdxBuffer});
            std::memcpy(idx_ptr, indices.data(), indices.size_bytes());
            idx_ptr = static_cast<char*>(idx_ptr) + indices.size_bytes();
        }
        backend->render_context->device()->unmap(buffers.vertex_buffer.get());
        backend->render_context->device()->unmap(buffers.index_buffer.get());
    }

    auto* render_command = backend->render_context->render_command();

    // Set pipeline
    render_command->bind_pipeline({.pipeline = backend->pipeline.get(), .bind_point = orion::PipelineBindPoint::Graphics});

    // Bind vertex buffer
    render_command->bind_vertex_buffer({.vertex_buffer = buffers.vertex_buffer.get(), .offset = 0});

    // Bind index buffer
    render_command->bind_index_buffer({.index_buffer = buffers.index_buffer.get(), .offset = 0, .index_type = orion::IndexType::Uint16});

    // Bind font descriptor
    render_command->bind_descriptor({
        .bind_point = orion::PipelineBindPoint::Graphics,
        .pipeline_layout = backend->pipeline_layout.get(),
        .index = 0,
        .descriptor = backend->font_descriptor.get(),
    });

    // Set push constants
    const auto scale = vec2(2.0f / draw_data->DisplaySize.x, 2.0f / draw_data->DisplaySize.y);
    const auto translation = vec2(-1.0f - draw_data->DisplayPos.x * scale[0], -1.0f - draw_data->DisplayPos.y * scale[1]);
    ImGui_ImplOrion_Constants constants{
        .scale = scale,
        .translation = translation,
    };
    render_command->push_constants({
        .pipeline_layout = backend->pipeline_layout.get(),
        .shader_stages = orion::ShaderStageFlags::Vertex,
        .offset = 0,
        .size = sizeof(constants),
        .values = &constants,
    });

    // Set viewport
    render_command->set_viewports({
        .position = {},
        .size = {draw_data->DisplaySize.x, draw_data->DisplaySize.y},
        .depth = {0.f, 1.f},
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

            // Set scissor
            render_command->set_scissors({
                .offset = {static_cast<int>(clip_min.x), static_cast<int>(clip_min.y)},
                .size = {static_cast<uint32_t>(clip_max.x - clip_min.x), static_cast<uint32_t>(clip_max.y - clip_min.y)},
            });

            render_command->draw_indexed({
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

void ImGui_ImplOrion_OnEvent(const orion::WindowEvent& event)
{
    auto& io = ImGui::GetIO();
    if (const auto* keydown = event.get_if<KeyDown>()) {
        io.AddKeyEvent(ImGui_ImplOrion_KeycodeToImGuiKey(keydown->key), true);
    } else if (const auto* keyup = event.get_if<KeyUp>()) {
        io.AddKeyEvent(ImGui_ImplOrion_KeycodeToImGuiKey(keyup->key), false);
    } else if (const auto* mousedown = event.get_if<MouseButtonDown>()) {
        if (const auto imgui_button = ImGui_ImplOrion_MouseButtonToImGuiButton(mousedown->button); imgui_button != -1) {
            io.AddMouseButtonEvent(imgui_button, true);
        }
    } else if (const auto* mouseup = event.get_if<MouseButtonUp>()) {
        if (const auto imgui_button = ImGui_ImplOrion_MouseButtonToImGuiButton(mouseup->button); imgui_button != -1) {
            io.AddMouseButtonEvent(imgui_button, false);
        }
    } else if (const auto* mousemove = event.get_if<MouseMove>()) {
        io.AddMousePosEvent(static_cast<float>(mousemove->position.x()), static_cast<float>(mousemove->position.y()));
    } else if (const auto* mousescroll = event.get_if<MouseScroll>()) {
        io.AddMouseWheelEvent(0.f, static_cast<float>(mousescroll->delta));
    }
}
