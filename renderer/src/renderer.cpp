#include "orion-renderer/renderer.h"

#include "orion-core/window.h"
#include "orion-renderer/mesh.h"
#include "orion-utils/assertion.h"

#include <array>

#ifndef ORION_RENDERER_LOG_LEVEL
    #define ORION_RENDERER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    namespace
    {
        std::uint32_t select_device_type(
            std::span<const PhysicalDeviceDesc> physical_devices,
            PhysicalDeviceType expected)
        {
            if (auto iter = std::ranges::find_if(physical_devices, check_device_type(expected));
                iter != physical_devices.end()) {
                return iter->index;
            }
            return UINT32_MAX;
        }
    } // namespace

    std::uint32_t select_discrete(std::span<const PhysicalDeviceDesc> physical_devices)
    {
        return select_device_type(physical_devices, PhysicalDeviceType::Discrete);
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
        , render_backend_(create_backend())
        , render_device_(create_device(desc.device_select_fn))
        , swapchain_(create_swapchain(desc.window))
        , render_pass_(create_render_pass())
        , render_target_(create_render_target(desc.window->size()))
        , graphics_pipeline_(create_graphics_pipeline())
        , graphics_command_pool_(create_command_pool(CommandQueueType::Graphics))
        , transfer_command_pool_(create_command_pool(CommandQueueType::Transfer))
        , render_command_(create_render_command())
        , descriptor_pool_(create_descriptor_pool())
        , descriptor_set_(create_descriptor_set())
        , render_area_(desc.window->size())
    {
        register_resize_callbacks(desc.window);

        SPDLOG_LOGGER_DEBUG(logger(), "Render backend {} initialized.", backend()->name());
        SPDLOG_LOGGER_DEBUG(logger(), "Renderer initialized.");
    }

    void Renderer::begin_frame()
    {
        device()->wait(render_submission_);
        render_command_.reset();

        auto* begin_frame = render_command_.add_command<CmdBeginFrame>({});
        begin_frame->render_pass = render_pass_;
        begin_frame->render_target = render_target_;
        begin_frame->render_area = render_area_;
        begin_frame->clear_color = clear_color_;
    }

    void Renderer::end_frame()
    {
        render_command_.add_command<CmdEndFrame>({});

        const auto desc = SubmitDesc{
            .command_buffer = &render_command_,
            .queue_type = CommandQueueType::Graphics,
            .existing = render_submission_,
        };
        render_submission_ = device()->submit(desc);
    }

    void Renderer::present()
    {
        device()->present(swapchain_, render_submission_);
    }

    std::span<const DescriptorSetLayout> Renderer::descriptor_layouts()
    {
        static const std::array descriptor_layout{
            DescriptorSetLayout({
                DescriptorBinding{.type = DescriptorType::ConstantBuffer, .shader_stages = ShaderStage::Vertex, .count = 1},
            }),
        };
        return descriptor_layout;
    }

    std::unique_ptr<RenderBackend> Renderer::create_backend() const
    {
        ORION_ASSERT(backend_module_.platform_module() != nullptr);

        SPDLOG_LOGGER_TRACE(logger(), "Initializing render backend...");

        // Load the factory function
        auto pfn_create_backend = backend_module_.load_symbol<RenderBackend*(void)>("create_render_backend");
        SPDLOG_LOGGER_TRACE(logger(), "Loaded create_render_backend() (at: {})", fmt::ptr(pfn_create_backend));

        // Create the backend
        auto render_backend = std::unique_ptr<RenderBackend>(pfn_create_backend());
        if (!render_backend) {
            throw std::runtime_error("Failed to create render backend");
        }
        return render_backend;
    }

    uint32_t Renderer::select_physical_device(pfnSelectPhysicalDevice device_select_fn) const
    {
        ORION_ASSERT(backend() != nullptr);
        // Get the physical devices
        auto physical_devices = backend()->enumerate_physical_devices();
        SPDLOG_LOGGER_DEBUG(logger(), "Found {} physical device(s):", physical_devices.size());
        for (const auto& physical_device : physical_devices) {
            SPDLOG_LOGGER_DEBUG(logger(), "{}", physical_device.name);
            SPDLOG_LOGGER_DEBUG(logger(), "-- Index: {}", physical_device.index);
            SPDLOG_LOGGER_DEBUG(logger(), "-- Type: {}", physical_device.type);
        }

        // Select the physical device to use
        const auto physical_device_index =
            [device_select_fn, &physical_devices]() -> uint32_t {
            if (device_select_fn != nullptr) {
                return device_select_fn(physical_devices);
            }
            return 0;
        }();
        if (physical_device_index == UINT32_MAX) {
            throw std::runtime_error("Couldn't find a suitable physical device");
        }

        SPDLOG_LOGGER_INFO(logger(), "Using physical device index {}", physical_device_index);
        return physical_device_index;
    }

    std::unique_ptr<RenderDevice> Renderer::create_device(pfnSelectPhysicalDevice device_select_fn) const
    {
        ORION_ASSERT(backend() != nullptr);
        SPDLOG_LOGGER_TRACE(logger(), "Creating render device...");
        const auto physical_device_index = select_physical_device(device_select_fn);
        return backend()->create_device(physical_device_index);
    }

    SwapchainHandle Renderer::create_swapchain(Window* window) const
    {
        ORION_ASSERT(device() != nullptr);
        SPDLOG_LOGGER_TRACE(logger(), "Creating swapchain...");
        const auto desc = SwapchainDesc{
            .image_count = 2,
            .image_format = image_format,
            .image_size = window->size(),
        };
        return device()->create_swapchain(*window, desc);
    }

    RenderPassHandle Renderer::create_render_pass() const
    {
        ORION_ASSERT(device() != nullptr);
        SPDLOG_LOGGER_TRACE(logger(), "Creating render pass...");
        const std::array color_attachments{
            orion::AttachmentDesc{
                .load_op = orion::AttachmentLoadOp::Clear,
                .store_op = orion::AttachmentStoreOp::Store,
                .initial_layout = orion::ImageLayout::Undefined,
                .layout = orion::ImageLayout::ColorAttachment,
                .final_layout = orion::ImageLayout::PresentSrc,
            },
        };
        return device()->create_render_pass({.color_attachments = color_attachments});
    }

    RenderTargetHandle Renderer::create_render_target(const Vector2_u& size)
    {
        ORION_ASSERT(device() != nullptr);
        ORION_ASSERT(swapchain_.is_valid());
        SPDLOG_LOGGER_TRACE(logger(), "Creating render target for swapchain...");
        const auto desc = RenderTargetDesc{
            .format = image_format,
            .render_pass = render_pass_,
            .size = size,
        };
        render_target_ = device()->create_render_target(swapchain_, desc);
    }

    void Renderer::register_resize_callbacks(Window* window)
    {
        ORION_ASSERT(window != nullptr);
        window->on_resize_end().subscribe([this](const auto& resize) {
            // Recreate swapchain
            {
                const auto desc = SwapchainDesc{
                    .image_count = 2,
                    .image_format = image_format,
                    .image_size = resize.size,
                };
                device()->recreate(swapchain_, desc);
            }

            // Recreate render target
            {
                const auto desc = RenderTargetDesc{
                    .format = image_format,
                    .render_pass = render_pass_,
                    .size = resize.size,
                };
                device()->recreate(render_target_, swapchain_, desc);
            }

            render_area_ = resize.size;
        });
    }

    ShaderModuleHandle Renderer::create_shader(const std::string& filepath, const ShaderStage& stage) const
    {
        ORION_ASSERT(device() != nullptr);
        const auto desc = ShaderCompileDesc{
            .source_file = filepath,
            .shader_type = stage,
            .object_type = backend()->shader_object_type(),
        };
        const auto result = shader_compiler_.compile(desc);
        return device()->create_shader_module({.byte_code = result.binary});
    }

    PipelineHandle Renderer::create_graphics_pipeline() const
    {
        ORION_ASSERT(device() != nullptr);
        SPDLOG_LOGGER_TRACE(logger(), "Creating graphics pipeline...");
        const std::array shaders{
            ShaderStageDesc{.module = create_shader(vertex_shader_path, ShaderStage::Vertex), .stage = ShaderStage::Vertex},
            ShaderStageDesc{.module = create_shader(fragment_shader_path, ShaderStage::Fragment), .stage = ShaderStage::Fragment},
        };

        const auto vertex_bindings = Vertex::vertex_bindings();

        const auto desc = GraphicsPipelineDesc{
            .shaders = shaders,
            .vertex_bindings = vertex_bindings,
            .descriptor_layouts = descriptor_layouts(),
            .input_assembly = input_assembly_,
            .rasterization = rasterization_,
            .render_pass = render_pass_};
        return device()->create_graphics_pipeline(desc);
    }

    CommandPoolHandle Renderer::create_command_pool(CommandQueueType queue_type) const
    {
        ORION_ASSERT(device() != nullptr);
        SPDLOG_LOGGER_TRACE(logger(), "Creating command pool...");
        return device()->create_command_pool(CommandPoolDesc{.queue_type = queue_type});
    }

    CommandBuffer Renderer::create_render_command() const
    {
        ORION_ASSERT(device() != nullptr);
        ORION_ASSERT(graphics_command_pool_.is_valid());
        SPDLOG_LOGGER_TRACE(logger(), "Creating render command buffer...");
        auto handle = device()->create_command_buffer(CommandBufferDesc{
            .command_pool = graphics_command_pool_,
        });
        return {handle, std::make_unique<LinearCommandAllocator>(render_command_size)};
    }

    DescriptorPoolHandle Renderer::create_descriptor_pool() const
    {
        ORION_ASSERT(device() != nullptr);
        SPDLOG_LOGGER_TRACE(logger(), "Creating descriptor pool...");
        static const std::array pool_sizes{
            DescriptorPoolSize{.type = DescriptorType::ConstantBuffer, .count = 1},
        };
        const auto desc = DescriptorPoolDesc{
            .max_sets = 1,
            .pool_sizes = pool_sizes};
        return device()->create_descriptor_pool(desc);
    }

    DescriptorSetHandle Renderer::create_descriptor_set() const
    {
        ORION_ASSERT(device() != nullptr);
        ORION_ASSERT(descriptor_pool_.is_valid());
        SPDLOG_LOGGER_TRACE(logger(), "Creating descriptor sets...");
        const auto& layout = descriptor_layouts()[0];
        const auto desc = DescriptorSetDesc{
            .descriptor_pool = descriptor_pool_,
            .layout = &layout,
        };
        return device()->create_descriptor_set(desc);
    }

    spdlog::logger* Renderer::logger()
    {
        static const auto renderer_logger = create_logger("orion-renderer", static_cast<spdlog::level::level_enum>(ORION_RENDERER_LOG_LEVEL));
        return renderer_logger.get();
    }
} // namespace orion
