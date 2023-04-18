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
        SPDLOG_INFO("Vertex shader binary size: {}", vs_compile_result.binary.size());
        const auto vs_module = device->create_shader_module({.byte_code = vs_compile_result.binary});

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
        SPDLOG_INFO("Fragment shader binary size: {}", fs_compile_result.binary.size());
        const auto fs_module = device->create_shader_module({.byte_code = vs_compile_result.binary});
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
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
