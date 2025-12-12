#include "orion/renderer/renderer.hpp"

#include "orion/rhi/rhi.hpp"

#include "orion/platform/window.hpp"

#include "orion/assert.hpp"
#include "orion/log.hpp"

#include "orion/config.h"

#include <array>
#include <fstream>
#include <vector>

namespace orion
{
    namespace
    {
        constexpr auto swapchain_format = RHIFormat::B8G8R8A8_Unorm_Srgb;
        constexpr auto swapchain_image_count = 2;

        std::unique_ptr<RHIInstance> rhi;
        std::unique_ptr<RHIDevice> device;
        std::unique_ptr<RHICommandQueue> command_queue;
        std::unique_ptr<RHISwapchain> swapchain;
        RHIPipeline pipeline;
        std::array<RHISemaphore, swapchain_image_count> image_acquired_semaphores;
        RHIFence render_finished_fence;

        auto load_shader(const char* path)
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            const auto size = file.tellg();
            file.seekg(std::ios::beg);
            std::vector<std::byte> buffer(size);
            file.read(reinterpret_cast<char*>(buffer.data()), size);
            return buffer;
        }
    } // namespace

    bool Renderer::init(const struct Window* window)
    {
        ORION_ASSERT(rhi == nullptr, "Renderer has already been initialized");
        ORION_ASSERT(window != nullptr, "Window must not be nullptr");

        int window_width, window_height;
        platform_window_get_size(window, &window_width, &window_height);
        ORION_ASSERT(window_width > 0, "Window width must be greater than 0");
        ORION_ASSERT(window_height > 0, "Window height must be greater than 0");

        rhi = rhi_create_instance();
        if (rhi == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHIInstance");
            return false;
        }

        device = rhi->create_device();
        if (rhi == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHIDevice");
            return false;
        }

        command_queue = device->create_command_queue({.type = RHICommandQueueType::Graphics});
        if (command_queue == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHICommandQueue");
            return false;
        }

        swapchain = device->create_swapchain({
            .window = window,
            .queue = command_queue.get(),
            .width = static_cast<std::uint32_t>(window_width),
            .height = static_cast<std::uint32_t>(window_height),
            .format = swapchain_format,
            .image_count = swapchain_image_count,
        });
        if (swapchain == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHISwapchain");
            return false;
        }

        pipeline = device->create_graphics_pipeline({
            .VS = load_shader(ORION_SHADER_DIR "/vertex.spv"),
            .FS = load_shader(ORION_SHADER_DIR "/fragment.spv"),
            .vertex_bindings = {},
            .input_assembly = {.topology = RHIPrimitiveTopology::TriangleList},
            .rasterizer = {
                .fill_mode = RHIFillMode::Solid,
                .cull_mode = RHICullMode::Back,
                .front_face = RHIFrontFace::CounterClockwise,
            },
            .depth_stencil = {
                .depth_enable = false,
            },
            .blend = {
                .render_targets = {{
                    RHIRenderTargetBlendDesc{.blend_enable = false, .color_write_mask = RHIColorWriteFlags::All},
                }},
            },
            .rtv_formats = {{swapchain_format}},
        });
        if (!pipeline.is_valid()) {
            ORION_CORE_LOG_ERROR("Failed to create RHIPipeline");
            return false;
        }

        for (int i = 0; i < swapchain_image_count; ++i) {
            image_acquired_semaphores[i] = device->create_semaphore({});
            if (!image_acquired_semaphores[i].is_valid()) {
                ORION_CORE_LOG_ERROR("Failed to create RHISemaphore");
                return false;
            }
        }

        render_finished_fence = device->create_fence({.create_signaled = true});
        if (!render_finished_fence.is_valid()) {
            ORION_CORE_LOG_ERROR("Failed to create RHIFence");
            return false;
        }

        return true;
    }

    void Renderer::shutdown()
    {
        ORION_ASSERT(rhi != nullptr, "Renderer has not been initialized or has already been shut down");

        device->destroy(render_finished_fence);
        for (int i = 0; i < swapchain_image_count; ++i) {
            device->destroy(image_acquired_semaphores[i]);
        }
        device->destroy(pipeline);
        swapchain = nullptr;
        command_queue = nullptr;
        device = nullptr;
        rhi = nullptr;
    }
} // namespace orion
