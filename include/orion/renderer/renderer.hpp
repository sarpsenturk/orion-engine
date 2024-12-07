#pragma once

#include "orion/renderer/color.hpp"
#include "orion/renderer/camera.hpp"

#include "orion/renderapi/render_backend.hpp"
#include "orion/renderapi/render_command.hpp"
#include "orion/renderapi/render_device.hpp"
#include "orion/renderapi/render_queue.hpp"

#include "orion/math/matrix/matrix4.hpp"

#include <cstdint>

namespace orion
{
    struct RendererDesc {
        std::unique_ptr<RenderBackend> render_backend;
        std::uint32_t width;
        std::uint32_t height;
    };

    class Renderer
    {
    public:
        explicit Renderer(RendererDesc desc);

        void begin_frame(const Camera& camera);
        void end_frame();

        void present(Swapchain* swapchain, std::uint32_t swapchain_width, std::uint32_t swapchain_height);

        [[nodiscard]] RenderBackend* render_backend() const { return render_backend_.get(); }
        [[nodiscard]] RenderDevice* render_device() const { return render_device_.get(); }
        [[nodiscard]] CommandQueue* graphics_queue() const { return graphics_queue_.get(); }
        [[nodiscard]] CommandList* draw_command() const { return draw_command_.get(); }
        [[nodiscard]] std::uint32_t render_width() const { return render_width_; }
        [[nodiscard]] std::uint32_t render_height() const { return render_height_; }

    private:
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
        std::unique_ptr<CommandQueue> graphics_queue_;
        std::unique_ptr<CommandAllocator> command_allocator_;
        std::unique_ptr<CommandList> draw_command_;
        std::unique_ptr<CommandList> present_command_;

        DescriptorSetLayoutHandle fullscreen_descriptor_layout_;
        PipelineLayoutHandle fullscreen_pipeline_layout_;
        PipelineHandle fullscreen_pipeline_;
        DescriptorPoolHandle fullscreen_descriptor_pool_;
        DescriptorSetHandle fullscreen_descriptor_set_;

        std::uint32_t render_width_;
        std::uint32_t render_height_;
        ImageHandle render_image_;
        SamplerHandle render_image_sampler_;
        ImageViewHandle render_image_view_;

        SemaphoreHandle frame_semaphore_;
        FenceHandle frame_fence_;
    };
} // namespace orion
