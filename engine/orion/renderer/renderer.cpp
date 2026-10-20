#include "renderer.hpp"

#include "orion/core/window.hpp"
#include "orion/lib/array.hpp"

namespace orion
{
    Renderer::Renderer(const RendererDesc& desc)
        : m_device(gpu::create_device({}))
        , m_swapchain(gpu::create_swapchain(
              m_device.get(),
              gpu::SwapchainDesc{
                  .native_window_handle = desc.window->handle(),
                  .width                = static_cast<std::uint32_t>(desc.window->width()),
                  .height               = static_cast<std::uint32_t>(desc.window->height()),
                  .image_count          = swapchain_image_count,
                  .image_format         = swapchain_format,
              }))
        , m_geometry_manager(m_device.get())
        , m_frame_data(make_array<frames_in_flight>([this]() {
            return FrameData{
                .command_allocator = gpu::create_command_allocator(m_device.get(), gpu::Queue::Graphics),
                .delete_queue      = DeletionQueue{m_device.get()},
            };
        }))
    {
    }

    Renderer::~Renderer()
    {
        // Wait until the GPU is idle before tearing down Renderer
        gpu::wait_idle(m_device.get());

        for (auto& frame_data : m_frame_data) {
            frame_data.delete_queue.flush();
            gpu::destroy_command_allocator(m_device.get(), frame_data.command_allocator);
        }
        gpu::destroy_swapchain(m_device.get(), m_swapchain);
    }

    std::optional<RenderFrame> Renderer::begin_frame()
    {
        auto& frame_data = current_frame_data();

        // Wait until the previous submission for this frame has completed
        gpu::wait_timeline(m_device.get(), frame_data.timeline_point);

        // Flush the deletion queue for this frame
        frame_data.delete_queue.flush();

        // Reset command allocator
        gpu::reset_command_allocator(m_device.get(), frame_data.command_allocator);

        if (m_swapchain_out_of_date) {
            return std::nullopt;
        }
        auto acquired_frame = gpu::acquire_swapchain_frame(m_device.get(), m_swapchain);
        if (!acquired_frame) {
            m_swapchain_out_of_date = true;
            return std::nullopt;
        }
        frame_data.swapchain_frame = *acquired_frame;

        // Reset the render graph
        m_render_graph.reset();

        // Get swapchain image extent
        const auto swapchain_extent = gpu::get_image_extent_2d(frame_data.swapchain_frame.image);

        // Import swapchain image as a render graph resource
        auto backbuffer = m_render_graph.import_texture(
            "Backbuffer",
            RGTextureDesc{
                .width  = swapchain_extent.width,
                .height = swapchain_extent.height,
                .depth  = 1u,
                .format = swapchain_format,
            },
            RGTextureImportDesc{
                .image         = frame_data.swapchain_frame.image,
                .image_view    = frame_data.swapchain_frame.image_view,
                .initial_state = {
                    .stages = gpu::PipelineStage::ColorAttachmentOutput,
                    .access = gpu::Access::None,
                    .layout = gpu::ImageLayout::Undefined,
                },
                .final_state = {
                    .stages = gpu::PipelineStage::AllCommands,
                    .access = gpu::Access::None,
                    .layout = gpu::ImageLayout::Present,
                },
            });

        return RenderFrame{m_render_graph, backbuffer, swapchain_extent};
    }

    void Renderer::end_frame()
    {
        auto& frame_data     = current_frame_data();
        auto* command_buffer = gpu::begin_command_buffer(m_device.get(), frame_data.command_allocator);

        // Compile the render graph
        m_render_graph.compile();

        // Execute the render graph
        m_render_graph.execute(command_buffer);

        // Submit commands for execution
        frame_data.timeline_point = gpu::submit(
            m_device.get(),
            gpu::Queue::Graphics,
            gpu::Submit{
                .command_buffers = {{command_buffer}},
                .swapchain       = m_swapchain,
                .swapchain_frame = &frame_data.swapchain_frame,
            });

        m_geometry_manager.retire_meshes(frame_data.delete_queue);

        // Present the swapchain frame
        gpu::present(m_device.get(), m_swapchain, frame_data.swapchain_frame);

        // Increment the frame counter for the next frame
        ++m_frame_counter;
    }

    void Renderer::resize_swapchain(std::uint32_t width, std::uint32_t height)
    {
        // Wait until the GPU is idle so we can safely resize the swapchain
        gpu::wait_idle(m_device.get());
        gpu::resize_swapchain(m_device.get(), m_swapchain, width, height);
        m_swapchain_out_of_date = false;
    }
} // namespace orion
