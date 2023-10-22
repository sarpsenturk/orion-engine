#include "orion-renderer/renderer.h"

#include "orion-renderer/shader_compiler.h"

#include "orion-renderapi/config.h"

#include "orion-core/window.h"

#include "orion-utils/assertion.h"

#include <algorithm>

#ifndef ORION_RENDERER_LOG_LEVEL
    #define ORION_RENDERER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    namespace
    {
        constexpr auto present_hlsl = R"hlsl(
struct VsOut {
    float4 position : SV_Position;
};

VsOut vs_main()
{
    VsOut output;
    output.position = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}


float4 fs_main() : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
)hlsl";
    }

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

    Renderer::Renderer(const RendererDesc& desc)
        : backend_module_(desc.backend_module)
        , render_backend_(create_render_backend())
        , render_device_(create_render_device(nullptr))
        , render_size_(desc.render_size)
        , shader_manager_(device())
        , present_pipeline_(create_present_pipeline())
        , present_render_pass_(create_present_render_pass())
        , frames_(create_frame_data())
    {
    }

    void Renderer::begin()
    {
        // Get current frame
        auto& frame = current_frame();

        // Reset command pool
        device()->reset_command_pool(frame.command_pool);

        // Get and reset command list
        auto& render_command = frame.render_command;
        render_command.reset();

        // Begin new frame
        render_command.begin();
    }

    void Renderer::end()
    {
        // Get current frame
        auto& frame = current_frame();

        // Get and end render command
        auto& render_command = frame.render_command;
        render_command.end();

        // Get command buffer and compile command list into it
        auto command_buffer = frame.command_buffer;
        device()->compile_commands(frame.command_buffer, render_command.commands());

        // Submit command buffer
        device()->submit({
            .queue_type = CommandQueueType::Graphics,
            .command_buffers = {&command_buffer, 1},
            .signal_semaphores = {&frame.render_semaphore, 1},
            .fence = frame.render_fence,
        });

        // Advance current frame index
        advance_frame();
    }

    void Renderer::present(SwapchainHandle swapchain)
    {
    }

    spdlog::logger* Renderer::logger()
    {
        static const auto logger = create_logger("orion-renderer", ORION_RENDERER_LOG_LEVEL);
        return logger.get();
    }

    AttachmentList Renderer::present_attachment_list()
    {
        static const auto color_attachments = std::array{
            AttachmentDesc{
                .format = Format::B8G8R8A8_Srgb,
                .load_op = AttachmentLoadOp::DontCare,
                .store_op = AttachmentStoreOp::Store,
                .initial_layout = ImageLayout::Undefined,
                .layout = ImageLayout::ColorAttachment,
                .final_layout = ImageLayout::PresentSrc,
            },
        };
        static const auto input_attachments = std::array{
            AttachmentDesc{
                .format = Format::B8G8R8A8_Srgb,
                .load_op = AttachmentLoadOp::DontCare,
                .store_op = AttachmentStoreOp::Store,
                .initial_layout = ImageLayout::Undefined,
                .layout = ImageLayout::ShaderReadOnly,
                .final_layout = ImageLayout::ColorAttachment,
            },
        };
        return {.color_attachments = color_attachments, .input_attachments = input_attachments};
    }

    void Renderer::advance_frame() noexcept
    {
        previous_frame_index_ = current_frame_index_;
        current_frame_index_ = (current_frame_index_ + 1) % frames_in_flight;
    }

    std::unique_ptr<RenderBackend> Renderer::create_render_backend() const
    {
        ORION_ASSERT(backend_module_.is_loaded());

        auto* fn_create_render_backend = backend_module_.load_symbol<pfnCreateRenderBackend>("create_render_backend");
        if (fn_create_render_backend == nullptr) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to load function 'create_render_backend' from module {}", backend_module_.filename());
            throw std::runtime_error("failed to load create_render_backend()");
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Symbol create_render_backend() loaded at {}", fmt::ptr(fn_create_render_backend));

        RenderBackend* render_backend = fn_create_render_backend();
        if (render_backend == nullptr) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to create render backend");
            throw std::runtime_error("failed to create render backend");
        }
        SPDLOG_LOGGER_DEBUG(logger(), "Render backend created");

        return std::unique_ptr<RenderBackend>{render_backend};
    }

    std::unique_ptr<RenderDevice> Renderer::create_render_device(pfnSelectPhysicalDevice device_select_fn) const
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
        SPDLOG_LOGGER_INFO(logger(), "Using physical device with index {}", physical_device_index);

        auto render_device = render_backend_->create_device(physical_device_index);
        SPDLOG_LOGGER_DEBUG(logger(), "Render device created");
        return render_device;
    }

    PipelineHandle Renderer::create_present_pipeline() const
    {
        auto vertex_shader = device()->to_unique(shader_manager_.create_vs(present_hlsl));
        auto fragment_shader = device()->to_unique(shader_manager_.create_fs(present_hlsl));
        const auto shaders = std::array{
            ShaderStageDesc{.module = vertex_shader.get(), .stage = ShaderStageFlags::Vertex, .entry_point = ShaderManager::vs_entry_point},
            ShaderStageDesc{.module = fragment_shader.get(), .stage = ShaderStageFlags::Fragment, .entry_point = ShaderManager::fs_entry_point},
        };
        const auto blend_attachments = std::array{
            BlendAttachmentDesc{
                .enable_blend = true,
                .src_blend = BlendFactor::One,
                .dst_blend = BlendFactor::Zero,
                .blend_op = BlendOp::Add,
                .src_blend_alpha = BlendFactor::One,
                .dst_blend_alpha = BlendFactor::Zero,
                .blend_op_alpha = BlendOp::Add,
                .color_component_flags = ColorComponentFlags::All,
            },
        };
        const auto color_blend = ColorBlendDesc{
            .enable_logic_op = false,
            .attachments = blend_attachments,
            .blend_constants = {1.f, 1.f, 1.f, 1.f},
        };
        return device()->create_graphics_pipeline({
            .shaders = shaders,
            .vertex_bindings = {},    // No vertex input, vertices generated by vertex shader
            .descriptor_layouts = {}, // TODO: Set descriptor layout
            .input_assembly = {
                .topology = PrimitiveTopology::TriangleList,
            },
            .rasterization = {
                .fill_mode = FillMode::Solid,
                .cull_mode = CullMode::Back,
                .front_face = FrontFace::CounterClockWise,
            },
            .color_blend = color_blend,
            .attachment_list = present_attachment_list(),
        });
    }

    RenderPassHandle Renderer::create_present_render_pass() const
    {
        return device()->create_render_pass({});
    }

    Renderer::FrameDataArr Renderer::create_frame_data() const
    {
        FrameDataArr frame_data;
        auto generate_frame_data = [this]() -> FrameData {
            auto image = device()->create_image({
                .type = ImageType::Image2D,
                .format = Format::B8G8R8A8_Srgb,
                .size = vec3(render_size_, 1u),
                .tiling = ImageTiling::Optimal,
                .usage = ImageUsageFlags::ColorAttachment | ImageUsageFlags::InputAttachment,
            });
            auto image_view = device()->create_image_view({
                .image = image,
                .type = ImageViewType::View2D,
                .format = Format::B8G8R8A8_Srgb,
            });
            const auto color_attachments = std::array{
                AttachmentDesc{
                    .format = Format::B8G8R8A8_Srgb,
                    .load_op = AttachmentLoadOp::Clear,
                    .store_op = AttachmentStoreOp::Store,
                    .initial_layout = ImageLayout::Undefined,
                    .layout = ImageLayout::ColorAttachment,
                    .final_layout = ImageLayout::ShaderReadOnly,
                },
            };
            auto render_target = device()->create_framebuffer({
                .attachment_list = AttachmentList{.color_attachments = color_attachments},
                .image_views = {&image_view, 1},
                .size = render_size_,
            });
            auto command_pool = device()->create_command_pool({.queue_type = CommandQueueType::Graphics});
            return {
                .render_image = image,
                .render_image_view = image_view,
                .render_target = render_target,
                .render_command = CommandList{render_command_size},
                .command_pool = command_pool,
                .command_buffer = device()->create_command_buffer({.command_pool = command_pool}),
                .render_fence = device()->create_fence(true),
                .render_semaphore = device()->create_semaphore(),
            };
        };
        std::generate(frame_data.begin(), frame_data.end(), generate_frame_data);
        return frame_data;
    }
} // namespace orion
