#include <orion-core/window.h>
#include <orion-engine/orion-engine.h>
#include <orion-renderer/renderer.h>
#include <orion-renderer/shader_compiler.h>
#include <spdlog/spdlog.h>

class SandboxApp : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Orion Sandbox", .position = {400, 200}, .size = {800, 600}})
        , renderer_(ORION_VULKAN_MODULE)
    {
        // Get the render device
        auto device = renderer_.device();

        // Create swapchain
        swapchain_ = device->create_swapchain(window_, {.image_count = 2, .image_format = orion::Format::B8G8R8A8_SRGB, .image_size = window_.size()});

        // Create shader compiler
        auto shader_compiler = orion::ShaderCompiler();

        // Create graphics pipeline
        {
            // Create vertex shader module
            const auto vs_module = [device, &shader_compiler]() {
                // Compile vertex shader
                const auto vs_source = R"(
struct VsInput {
    float4 position : POSITION;
    float4 color : COLOR;
};

struct VsOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
};

VsOutput main(VsInput input)
{
    VsOutput output;
    output.position = input.position;
    output.color = input.color;
    return output;
}
)";
                const auto vs_compile_result = shader_compiler.compile({.shader_source = vs_source, .shader_type = orion::ShaderType::Vertex, .object_type = orion::ShaderObjectType::SpirV});
                return device->create_shader_module({.byte_code = vs_compile_result.binary});
            }();

            // Create fragment shader module
            const auto fs_module = [device, &shader_compiler]() {
                // Compile fragment shader
                const auto fs_source = R"(
struct FsInput {
    float4 position : SV_Position;
    float4 color : COLOR;
};

float4 main(FsInput input) : SV_Target
{
    return input.color;
}
)";
                const auto fs_compile_result = shader_compiler.compile({.shader_source = fs_source, .shader_type = orion::ShaderType::Fragment, .object_type = orion::ShaderObjectType::SpirV});
                return device->create_shader_module({.byte_code = fs_compile_result.binary});
            }();

            // Pipeline shaders
            const std::array shaders{
                orion::ShaderStageDesc{.module = vs_module, .type = orion::ShaderType::Vertex},
                orion::ShaderStageDesc{.module = fs_module, .type = orion::ShaderType::Fragment},
            };

            // Pipeline vertex bindings
            const std::array vertex_bindings{
                orion::VertexBinding{{orion::VertexAttributeDesc{.name = "POSITION", .format = orion::Format::R32G32B32A32_FLOAT},
                                      orion::VertexAttributeDesc{.name = "COLOR", .format = orion::Format::R32G32B32A32_FLOAT}},
                                     orion::InputRate::Vertex}};

            // Create the graphics pipeline
            graphics_pipeline_ = device->create_graphics_pipeline(orion::GraphicsPipelineDesc{
                .shaders = shaders,
                .vertex_bindings = vertex_bindings,
                .render_target = swapchain_,
            });

            device->destroy(vs_module);
            device->destroy(fs_module);
        }
    }

private:
    void on_user_update() override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
    }

    [[nodiscard]] bool user_should_exit() const noexcept override
    {
        return window_.should_close();
    }

    orion::Window window_;
    orion::Renderer renderer_;
    orion::Swapchain swapchain_;
    orion::GraphicsPipeline graphics_pipeline_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
