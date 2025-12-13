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
        std::unique_ptr<RHICommandAllocator> command_allocator;
        std::unique_ptr<RHICommandList> command_list;
        RHISwapchain swapchain;
        std::array<RHIImage, swapchain_image_count> swapchain_images;
        std::array<RHIImageView, swapchain_image_count> swapchain_rtvs;
        RHIPipeline pipeline;
        std::array<RHISemaphore, swapchain_image_count> image_acquired_semaphores;
        RHIFence render_finished_fence;
        std::int32_t render_width;
        std::int32_t render_height;
        std::int32_t frame_index = 0;

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
        render_width = window_width;
        render_height = window_height;

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

        command_allocator = device->create_command_allocator({.type = RHICommandQueueType::Graphics});
        if (command_allocator == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHICommandAllocator");
            return false;
        }

        command_list = device->create_command_list({.command_allocator = command_allocator.get()});
        if (command_list == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHICommandList");
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
        if (!swapchain.is_valid()) {
            ORION_CORE_LOG_ERROR("Failed to create RHISwapchain");
            return false;
        }

        for (std::uint32_t i = 0; i < swapchain_image_count; ++i) {
            swapchain_images[i] = device->get_swapchain_image(swapchain, i);
            swapchain_rtvs[i] = device->create_render_target_view({.image = swapchain_images[i], .format = swapchain_format});
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
            device->destroy(swapchain_rtvs[i]);
        }
        device->destroy(pipeline);
        device->destroy(swapchain);
        command_list = nullptr;
        command_allocator = nullptr;
        command_queue = nullptr;
        device = nullptr;
        rhi = nullptr;
    }

    void Renderer::render()
    {
        // Wait for previous render to finish
        device->wait_for_fences({{render_finished_fence}}, true, UINT64_MAX);
        device->reset_fences({{render_finished_fence}});

        // Reset command list
        command_allocator->reset();
        command_list->reset();

        // Acquire swapchain image index
        const auto image_index = device->acquire_swapchain_image(swapchain, image_acquired_semaphores[frame_index], RHIFence::invalid());

        // Begin command list recording
        command_list->begin();

        // Transition swapchain image to render target
        command_list->pipeline_barrier({
            .transition_barriers = {{
                RHITransitionBarrier{
                    .image = swapchain_images[image_index],
                    .old_layout = RHIImageLayout::Undefined,
                    .new_layout = RHIImageLayout::RenderTarget,
                },
            }},
        });

        // Begin rendering
        command_list->begin_rendering({
            .render_width = static_cast<std::uint32_t>(render_width),
            .render_height = static_cast<std::uint32_t>(render_height),
            .rtvs = {{swapchain_rtvs[image_index]}},
            .rtv_clear = {1.0f, 0.0f, 1.0f, 1.0f},
        });

        // Bind graphics pipeline
        command_list->set_graphics_pipeline_state(pipeline);

        // Set viewport
        command_list->set_viewports({
            .first_viewport = 0,
            .viewports = {{
                RHIViewport{
                    .x = 0,
                    .y = 0,
                    .width = static_cast<float>(render_width),
                    .height = static_cast<float>(render_height),
                    .min_depth = 0.0f,
                    .max_depth = 1.0f,
                },
            }},
        });

        // Set scissor
        command_list->set_scissors({
            .scissors = {{
                RHIRect{
                    .left = 0,
                    .top = 0,
                    .right = static_cast<std::int32_t>(render_width),
                    .bottom = static_cast<std::int32_t>(render_height),
                },
            }},
        });

        // Issue draw call
        command_list->draw_instanced({.vertex_count = 3, .instance_count = 1, .first_vertex = 0, .first_instance = 0});

        // End rendering
        command_list->end_rendering();

        // End command list recording
        command_list->end();
    }
} // namespace orion
