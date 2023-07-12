#pragma once

#include "orion-core/config.h"
#include "orion-core/platform.h"
#include "orion-renderapi/render_backend.h"
#include "orion-renderer/config.h"
#include "shader_compiler.h"

#include <memory>
#include <span>
#include <spdlog/logger.h>

namespace orion
{
    using pfnSelectPhysicalDevice =
        std::uint32_t (*)(std::span<const PhysicalDeviceDesc>);

    std::uint32_t select_discrete(std::span<const PhysicalDeviceDesc>);

    const char* default_backend_module(Platform platform = current_platform);

    struct RendererDesc {
        const char* backend_module = default_backend_module();
        pfnSelectPhysicalDevice device_select_fn = nullptr;
        Window* window = nullptr;
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto device() const noexcept { return render_device_.get(); }

        void begin_frame();
        void end_frame();
        void present();

        static spdlog::logger* logger();

    private:
        static std::span<const DescriptorSetLayout> descriptor_layouts();

        [[nodiscard]] std::unique_ptr<RenderBackend> create_backend() const;
        [[nodiscard]] uint32_t select_physical_device(pfnSelectPhysicalDevice device_select_fn) const;
        [[nodiscard]] std::unique_ptr<RenderDevice> create_device(pfnSelectPhysicalDevice device_select_fn) const;
        [[nodiscard]] SwapchainHandle create_swapchain(Window* window) const;
        [[nodiscard]] RenderPassHandle create_render_pass() const;
        [[nodiscard]] RenderTargetHandle create_render_target(const Vector2_u& size);
        [[nodiscard]] ShaderModuleHandle create_shader(const std::string& filepath, const ShaderStage& stage) const;
        [[nodiscard]] PipelineHandle create_graphics_pipeline() const;
        [[nodiscard]] CommandPoolHandle create_command_pool(CommandQueueType queue_type) const;
        [[nodiscard]] CommandBuffer create_render_command() const;
        [[nodiscard]] DescriptorPoolHandle create_descriptor_pool() const;
        [[nodiscard]] DescriptorSetHandle create_descriptor_set() const;
        void register_resize_callbacks(Window* window);

        static constexpr auto image_format = Format::B8G8R8A8_Srgb;
        static constexpr auto vertex_shader_path = ORION_SHADER_DIR "/vert.hlsl";
        static constexpr auto fragment_shader_path = ORION_SHADER_DIR "/frag.hlsl";
        static constexpr auto render_command_size = std::size_t{2048};

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;

        ShaderCompiler shader_compiler_;

        InputAssemblyDesc input_assembly_ = {
            .topology = PrimitiveTopology::TriangleList,
        };
        RasterizationDesc rasterization_ = {
            .fill_mode = FillMode::Solid,
            .cull_mode = CullMode::Back,
            .front_face = FrontFace::ClockWise,
        };
        Vector2_u render_area_;
        Vector4_f clear_color_ = {1.f, 0.f, 1.f, 1.f};

        SwapchainHandle swapchain_;
        RenderPassHandle render_pass_;
        RenderTargetHandle render_target_;

        PipelineHandle graphics_pipeline_;

        CommandPoolHandle graphics_command_pool_;
        CommandPoolHandle transfer_command_pool_;
        CommandBuffer render_command_;

        DescriptorPoolHandle descriptor_pool_;
        DescriptorSetHandle descriptor_set_;

        SubmissionHandle render_submission_;
    };
} // namespace orion
