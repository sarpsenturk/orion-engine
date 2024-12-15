#include "orion/renderer/renderer.hpp"

#include <stdexcept>

static constexpr auto fullscreen_vertex_shader = R"hlsl(
struct VSOut {
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

VSOut main(uint id : SV_VertexID)
{
    VSOut output;
    output.uv = float2((id << 1) & 2, id & 2);
    output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}
)hlsl";

// vertex_id    uv      position
//  0           0,0     -1.0, -1.0, 1.0
//  1           2,0     3.0, -1.0, 1.0
//  2           0,2     -1.0, 3.0, 1.0

static constexpr auto fullscreen_pixel_shader = R"hlsl(
[[vk::binding(0, 0)]]
Texture2D _Texture : register(t0);
[[vk::binding(1, 0)]]
SamplerState _Sampler : register(s0);

float4 main(float2 uv : TEXCOORD) : SV_Target
{
    return _Texture.Sample(_Sampler, uv);
}
)hlsl";

namespace orion
{
    Renderer::Renderer(RendererDesc desc)
        : render_backend_(std::move(desc.render_backend))
        , render_device_(render_backend_->create_device(0)) // TODO: Allow user to select adapter
        , graphics_queue_(render_device_->create_command_queue())
        , command_allocator_(render_device_->create_command_allocator({}))
        , draw_command_(render_device_->create_command_list({.command_allocator = command_allocator_.get()}))
        , present_command_(render_device_->create_command_list({.command_allocator = command_allocator_.get()}))
        , render_width_(desc.width)
        , render_height_(desc.height)
    {
        // Create the bind group layout for fullscreen triangle
        fullscreen_bind_layout_ = render_device_->create_bind_group_layout({
            .bindings = {{
                DescriptorSetBindingDesc{.type = DescriptorType::ImageView, .size = 1},
                DescriptorSetBindingDesc{.type = DescriptorType::Sampler, .size = 1},
            }},
        });

        // Create the pipeline layout for fullscreen triangle
        fullscreen_pipeline_layout_ = render_device_->create_pipeline_layout({.bind_group_layouts = {{fullscreen_bind_layout_}}});

        // Create shader compiler
        auto shader_compiler = render_device_->create_shader_compiler();

        // Compile shaders for fullscreen rendering
        const auto vertex_shader = shader_compiler->compile({.source = fullscreen_vertex_shader, .type = ShaderType::Vertex});
        const auto pixel_shader = shader_compiler->compile({.source = fullscreen_pixel_shader, .type = ShaderType::Pixel});
        if (vertex_shader.empty() || pixel_shader.empty()) {
            throw std::runtime_error("Renderer: failed to compile fullscreen shaders");
        }

        // Create the pipeline for fullscreen triangle
        fullscreen_pipeline_ = render_device_->create_graphics_pipeline({
            .pipeline_layout = fullscreen_pipeline_layout_,
            .vertex_shader = vertex_shader,
            .pixel_shader = pixel_shader,
            .vertex_attributes = {},
            .primitive_topology = PrimitiveTopology::Triangle,
            .rasterizer = {
                .fill_mode = FillMode::Solid,
                .cull_mode = CullMode::Back,
                .front_face = FrontFace::ClockWise,
            },
            .blend = {
                .render_targets = {{
                    RenderTargetBlendDesc{
                        .blend_enable = false,
                        .src_blend = Blend::One,
                        .dst_blend = Blend::Zero,
                        .blend_op = BlendOp::Add,
                        .src_blend_alpha = Blend::One,
                        .dst_blend_alpha = Blend::Zero,
                        .blend_op_alpha = BlendOp::Add,
                        .color_write_mask = ColorWriteFlags::All,
                    },
                }},
                .blend_constants = {1.f, 1.f, 1.f, 1.f},
            },
            .render_target_formats = {{Format::B8G8R8A8_Unorm}},
        });

        // Create internal render image
        render_image_ = render_device_->create_image({
            .width = render_width_,
            .height = render_height_,
            .depth = 1,
            .format = Format::B8G8R8A8_Unorm,
            .type = ImageType::Image2D,
            .usage_flags = ImageUsageFlags::RenderTarget | ImageUsageFlags::ShaderResource,
        });

        // Create internal render image view
        render_image_view_ = render_device_->create_image_view({
            .image = render_image_,
            .type = ImageViewType::View2D,
        });

        // Create fullscreen image sampler
        render_image_sampler_ = render_device_->create_sampler({
            .filter = Filter::Nearest,
            .address_mode_u = SamplerAddressMode::Border,
            .address_mode_v = SamplerAddressMode::Border,
            .address_mode_w = SamplerAddressMode::Border,
            .mip_lod_bias = 0.f,
            .compare_op = CompareOp::Always,
            .min_lod = 0.f,
            .max_lod = 0.f,
        });

        // Create fullscreen bind group
        fullscreen_bind_group_ = render_device_->create_bind_group({
            .layout = fullscreen_bind_layout_,
            .views = {{
                ImageViewBindingDesc{.binding = 0, .image_view = render_image_view_},
            }},
            .samplers = {{
                SamplerBindingDesc{.binding = 1, .sampler = render_image_sampler_},
            }},
        });

        // Create frame sync objects
        frame_semaphore_ = render_device_->create_semaphore({});
        frame_fence_ = render_device_->create_fence({.signaled = false});
    }

    void Renderer::begin_frame(const Camera& camera)
    {
        // Reset all command lists
        command_allocator_->reset();

        // Begin command list recording
        draw_command_->begin();

        // Transition render image to render target
        draw_command_->transition_barrier({.image = render_image_, .before = ImageState::Unknown, .after = ImageState::RenderTarget});

        // Begin rendering
        draw_command_->begin_rendering({
            .render_targets = {{
                RenderAttachment{
                    .render_target = render_image_view_,
                    .clear_color = camera.clear_color().as_array(),
                },
            }},
            .render_area = {
                .width = static_cast<std::int32_t>(render_width_),
                .height = static_cast<std::int32_t>(render_height_),
            },
        });
    }

    void Renderer::end_frame()
    {
        // End rendering
        draw_command_->end_rendering();

        // End command list recording
        draw_command_->end();

        // Signal semaphore when rendering is finished
        graphics_queue_->signal(frame_semaphore_);

        // Submit rendering work to queue
        graphics_queue_->submit({{draw_command_.get()}}, FenceHandle::Invalid);
    }

    void Renderer::present(Swapchain* swapchain, std::uint32_t swapchain_width, std::uint32_t swapchain_height)
    {
        // Acquire render target from swapchain
        const auto render_target = swapchain->acquire_render_target();

        // Get the swapchain image associated with render target
        const auto image = swapchain->current_image();

        // Begin command list recording
        present_command_->begin();

        // Transition swapchain image to render target
        present_command_->transition_barrier({.image = image, .before = ImageState::Unknown, .after = ImageState::RenderTarget});

        // Transition render result to shader resource
        present_command_->transition_barrier({.image = render_image_, .before = ImageState::RenderTarget, .after = ImageState::ShaderResource});

        // Begin rendering to swapchain image
        present_command_->begin_rendering({
            .render_targets = {{
                RenderAttachment{.render_target = render_target, .clear_color = {0.f, 0.f, 0.f, 1.f}},
            }},
            .render_area = {
                .width = static_cast<std::int32_t>(swapchain_width),
                .height = static_cast<std::int32_t>(swapchain_height),
            },
        });

        // Set fullscreen pipeline
        present_command_->set_pipeline({.pipeline = fullscreen_pipeline_, .layout = fullscreen_pipeline_layout_});

        // Set viewport
        present_command_->set_viewports({
            .start_viewport = 0,
            .viewports = {
                {
                    Viewport{
                        .x = 0,
                        .y = 0,
                        .width = static_cast<float>(swapchain_width),
                        .height = static_cast<float>(swapchain_height),
                        .min_depth = 0.f,
                        .max_depth = 1.f,
                    },
                }},
        });

        // Set scissor
        present_command_->set_scissors({
            .start_scissor = 0,
            .scissors = {{
                Rect2D{
                    .x = 0,
                    .y = 0,
                    .width = static_cast<std::int32_t>(swapchain_width),
                    .height = static_cast<std::int32_t>(swapchain_height),
                },
            }},
        });

        // Set fullscreen bind group
        present_command_->set_bind_group({.index = 0, .bind_group = fullscreen_bind_group_, .pipeline_layout = fullscreen_pipeline_layout_});

        // Draw fullscreen triangle
        present_command_->draw_instanced({.vertex_count = 3, .instance_count = 1, .start_vertex = 0, .start_instance = 0});

        // End rendering
        present_command_->end_rendering();

        // Transition swapchain image to present image
        present_command_->transition_barrier({.image = image, .before = ImageState::RenderTarget, .after = ImageState::Present});

        // End command list recording
        present_command_->end();

        // Wait until render result is available
        graphics_queue_->wait(frame_semaphore_);

        // Submit present commands
        // Signal fence to show frame is finished
        graphics_queue_->submit({{present_command_.get()}}, frame_fence_);

        // Present swapchain image to the screen
        swapchain->present(true); // TODO: Make this customizable

        // Wait for frame to finish
        render_device_->wait_for_fence(frame_fence_);
    }
} // namespace orion
