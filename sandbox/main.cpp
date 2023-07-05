#include <orion-core/window.h>
#include <orion-engine/orion-engine.h>
#include <orion-math/vector/vector3.h>
#include <orion-renderapi/command.h>
#include <orion-renderer/renderer.h>
#include <orion-renderer/shader_compiler.h>
#include <spdlog/spdlog.h>

class SandboxApp : public orion::Application
{
public:
    struct Vertex {
        orion::Vector3_f position;
        orion::Vector4_f color;
    };

    static constexpr std::array vertices{
        Vertex{.position = {-.5f, .5f, 0.f}, .color = {1.f, 0.f, 0.f, 1.f}},
        Vertex{.position = {.5f, .5f, 0.f}, .color = {0.f, 1.f, 0.f, 1.f}},
        Vertex{.position = {.5f, -.5f, 0.f}, .color = {0.f, 0.f, 1.f, 1.f}},
        Vertex{.position = {-.5f, -.5f, 0.f}, .color = {.5f, 0.f, .5f, 1.f}},
    };

    static constexpr std::array indices{0u, 1u, 2u, 2u, 3u, 0u};

    static constexpr auto swapchain_format = orion::Format::B8G8R8A8_Srgb;

    SandboxApp()
        : window_({.name = "Orion Sandbox", .position = window_position, .size = window_size})
        , renderer_(ORION_VULKAN_MODULE, &orion::select_discrete)
    {
        // Get the render device
        auto* device = renderer_.device();

        // Create swapchain
        swapchain_ = device->create_swapchain(window_,
                                              orion::SwapchainDesc{
                                                  .image_count = 2,
                                                  .image_format = swapchain_format,
                                                  .image_size = window_.size(),
                                              });

        // Create render pass
        {
            const std::array color_attachments{
                orion::AttachmentDesc{
                    .load_op = orion::AttachmentLoadOp::Clear,
                    .store_op = orion::AttachmentStoreOp::Store,
                    .initial_layout = orion::ImageLayout::Undefined,
                    .layout = orion::ImageLayout::ColorAttachment,
                    .final_layout = orion::ImageLayout::PresentSrc,
                },
            };
            render_pass_ = device->create_render_pass({.color_attachments = color_attachments});
        }

        // Create render target
        render_target_ = device->create_render_target(swapchain_,
                                                      orion::RenderTargetDesc{
                                                          .format = swapchain_format,
                                                          .render_pass = render_pass_,
                                                          .size = window_size,
                                                      });

        // Register callback on window resize to recreate swapchain
        window_.on_resize_end()
            .subscribe([device, this](const orion::events::WindowResizeEnd& resize_end) {
                device->recreate(swapchain_,
                                 orion::SwapchainDesc{
                                     .image_count = 2,
                                     .image_format = swapchain_format,
                                     .image_size = resize_end.size,
                                 });
                device->recreate(render_target_,
                                 swapchain_,
                                 orion::RenderTargetDesc{
                                     .format = swapchain_format,
                                     .render_pass = render_pass_,
                                     .size = resize_end.size,
                                 });
            });

        // Create descriptor set layout to be used in pipeline creation and descriptor set allocation
        const auto descriptor_layout = orion::DescriptorSetLayout({
            orion::DescriptorBinding{
                .type = orion::DescriptorType::ConstantBuffer,
                .shader_stages = orion::ShaderStage::Vertex,
                .count = 1,
            },
        });

        // Create graphics pipeline
        {
            // Create shader compiler
            auto shader_compiler = orion::ShaderCompiler();

            // Create vertex shader module
            const auto vs_module = [device, &shader_compiler]() {
                // Compile vertex shader
                const auto* vs_source = R"hlsl(
cbuffer CSceneBuffer {
    float4x4 view;
    float4x4 view_proj;
};

struct VsOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
};

VsOutput main(float3 position : POSITION, float4 color : COLOR)
{
    VsOutput output;
    output.position = float4(position, 1.0);
    output.color = color;
    return output;
}
)hlsl";
                const auto compile_desc = orion::ShaderCompileDesc{
                    .shader_source = vs_source,
                    .shader_type = orion::ShaderStage::Vertex,
                    .object_type = orion::ShaderObjectType::SpirV,
                };
                const auto vs_compile_result = shader_compiler.compile(compile_desc);
                return device->create_shader_module({.byte_code = vs_compile_result.binary});
            }();

            // Create fragment shader module
            const auto fs_module = [device, &shader_compiler]() {
                // Compile fragment shader
                const auto* fs_source = R"hlsl(
float4 main(float4 color : COLOR) : SV_Target
{
    return color;
}
)hlsl";
                const auto compile_desc = orion::ShaderCompileDesc{
                    .shader_source = fs_source,
                    .shader_type = orion::ShaderStage::Fragment,
                    .object_type = orion::ShaderObjectType::SpirV,
                };
                const auto fs_compile_result = shader_compiler.compile(compile_desc);
                return device->create_shader_module({.byte_code = fs_compile_result.binary});
            }();

            // Pipeline shaders
            const std::array shaders{
                orion::ShaderStageDesc{.module = vs_module, .stage = orion::ShaderStage::Vertex},
                orion::ShaderStageDesc{.module = fs_module, .stage = orion::ShaderStage::Fragment},
            };

            // Pipeline vertex bindings
            const std::array vertex_bindings{
                orion::VertexBinding{{orion::VertexAttributeDesc{.name = "POSITION", .format = orion::Format::R32G32B32_Float},
                                      orion::VertexAttributeDesc{.name = "COLOR", .format = orion::Format::R32G32B32A32_Float}},
                                     orion::InputRate::Vertex}};

            // Create the graphics pipeline
            graphics_pipeline_ = device->create_graphics_pipeline(orion::GraphicsPipelineDesc{
                .shaders = shaders,
                .vertex_bindings = vertex_bindings,
                .descriptor_layouts = {&descriptor_layout, 1},
                .render_pass = render_pass_,
            });

            // Destroy shader modules after pipeline creation
            device->destroy(vs_module);
            device->destroy(fs_module);
        }

        // Create command pools
        graphics_command_pool_ = device->create_command_pool({.queue_type = orion::CommandQueueType::Graphics});
        transfer_command_pool_ = device->create_command_pool({.queue_type = orion::CommandQueueType::Transfer});

        // Create staging buffer
        auto staging_buffer = device->create_buffer(orion::GPUBufferDesc{
            .size = sizeof(vertices) + sizeof(indices),
            .usage = orion::GPUBufferUsage::TransferSrc,
            .host_visible = true,
        });

        // Map staging buffer
        void* mapped_memory = device->map(staging_buffer);

        // Copy vertex & index data to staging buffer
        std::memcpy(mapped_memory, vertices.data(), sizeof(vertices));
        mapped_memory = static_cast<char*>(mapped_memory) + sizeof(vertices);
        std::memcpy(mapped_memory, indices.data(), sizeof(indices));

        // Create vertex buffer
        {
            const auto usage_flags = orion::GPUBufferUsageFlags::disjunction({orion::GPUBufferUsage::VertexBuffer,
                                                                              orion::GPUBufferUsage::TransferDst});
            const orion::GPUBufferDesc desc{
                .size = sizeof(vertices),
                .usage = usage_flags,
            };
            vertex_buffer_ = device->create_buffer(desc);
        }
        // Create index buffer
        {
            const auto usage_flags = orion::GPUBufferUsageFlags::disjunction({orion::GPUBufferUsage::IndexBuffer,
                                                                              orion::GPUBufferUsage::TransferDst});
            const orion::GPUBufferDesc desc{
                .size = sizeof(indices),
                .usage = usage_flags,
            };
            index_buffer_ = device->create_buffer(desc);
        }

        // Create command buffer to copy data from staging buffer
        auto copy_command_buffer = orion::CommandBuffer(
            device->create_command_buffer({.command_pool = transfer_command_pool_}),
            std::make_unique<orion::LinearCommandAllocator>(sizeof(orion::CmdBufferCopy) * 2));

        // Issue command to copy vertex data
        {
            auto* copy_cmd = copy_command_buffer.add_command<orion::CmdBufferCopy>({});
            copy_cmd->dst = vertex_buffer_;
            copy_cmd->dst_offset = 0;
            copy_cmd->src = staging_buffer;
            copy_cmd->src_offset = 0;
            copy_cmd->size = sizeof(vertices);
        }

        // Issue command to copy index data
        {
            auto* copy_cmd = copy_command_buffer.add_command<orion::CmdBufferCopy>({});
            copy_cmd->dst = index_buffer_;
            copy_cmd->dst_offset = 0;
            copy_cmd->src = staging_buffer;
            copy_cmd->src_offset = sizeof(vertices);
            copy_cmd->size = sizeof(indices);
        }

        // Submit the copy command
        auto submission = device->submit({
            .command_buffer = &copy_command_buffer,
            .queue_type = orion::CommandQueueType::Transfer,
        });

        // Unmap staging buffer
        device->unmap(staging_buffer);

        // Create the render command buffer
        render_command_ = orion::CommandBuffer(
            device->create_command_buffer({.command_pool = graphics_command_pool_}),
            std::make_unique<orion::LinearCommandAllocator>(render_command_size));

        // Create descriptor pool
        {
            const std::array pool_sizes{
                orion::DescriptorPoolSize{.type = orion::DescriptorType::ConstantBuffer, .count = 1},
            };
            descriptor_pool_ = device->create_descriptor_pool(orion::DescriptorPoolDesc{
                .max_sets = 1,
                .pool_sizes = pool_sizes,
            });
        }

        // Allocate descriptor set
        {
            descriptor_set_ = device->create_descriptor_set(orion::DescriptorSetDesc{
                .descriptor_pool = descriptor_pool_,
                .layout = &descriptor_layout,
            });
        }

        // Wait until copy is completed
        device->wait(submission);

        // Destroy copy submission, staging buffer and command buffer
        device->destroy(submission);
        device->destroy(staging_buffer);
        device->destroy(copy_command_buffer.handle());
    }

private:
    void on_user_update() override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
        // Do not render if window is minimized
        if (window_.is_minimized()) {
            return;
        }

        // Get device
        auto* device = renderer_.device();

        // Reset command buffer
        render_command_.reset();

        // Wait until last frame is finished
        // When called during the first frame the handle will be invalid, therefore
        device->wait(render_submission_);

        // Begin new frame
        {
            auto* begin_frame = render_command_.add_command<orion::CmdBeginFrame>({});
            begin_frame->render_pass = render_pass_;
            begin_frame->render_target = render_target_;
            begin_frame->render_area = window_.size();
            begin_frame->clear_color = {1.f, 0.f, 1.f, 1.f};
        }

        // Draw quad
        {
            auto* draw = render_command_.add_command<orion::CmdDrawIndexed>({});
            draw->vertex_buffer = vertex_buffer_;
            draw->index_buffer = index_buffer_;
            draw->graphics_pipeline = graphics_pipeline_;
            draw->viewport = {.position = {0.f, 0.f}, .size = orion::vector_cast<float>(window_.size())};
            draw->index_count = static_cast<std::uint32_t>(indices.size());
        }

        // End the frame
        render_command_.add_command<orion::CmdEndFrame>({});

        // Submit the commands
        render_submission_ = device->submit(orion::SubmitDesc{
            .command_buffer = &render_command_,
            .queue_type = orion::CommandQueueType::Graphics,
            .existing = render_submission_,
        });

        // Present
        device->present(swapchain_, render_submission_);
    }

    [[nodiscard]] bool user_should_exit() const noexcept override
    {
        return window_.should_close();
    }

    static constexpr auto window_position = orion::WindowPosition{400, 200};
    static constexpr auto window_size = orion::WindowSize{800, 600};
    static constexpr auto render_command_size = 2048;

    orion::Window window_;
    orion::Renderer renderer_;
    orion::SwapchainHandle swapchain_;
    orion::RenderPassHandle render_pass_;
    orion::RenderTargetHandle render_target_;
    orion::PipelineHandle graphics_pipeline_;
    orion::GPUBufferHandle vertex_buffer_;
    orion::GPUBufferHandle index_buffer_;
    orion::CommandPoolHandle graphics_command_pool_;
    orion::CommandPoolHandle transfer_command_pool_;
    orion::DescriptorPoolHandle descriptor_pool_;
    orion::DescriptorSetHandle descriptor_set_;
    orion::CommandBuffer render_command_;
    orion::SubmissionHandle render_submission_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
