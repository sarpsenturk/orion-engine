#pragma once

#include "orion-renderer/frame.h"

#include "orion-renderapi/device_resource.h"
#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_command.h"
#include "orion-renderapi/swapchain.h"

#include "orion-math/vector/vector2.h"

#include <vector>

namespace orion
{
    class RenderDevice;
    class Window;

    struct PresentDesc {
        frame_index_t frame_index;
        ImageViewHandle source_image;
        ImageLayout source_image_layout;
        std::span<const SemaphoreHandle> wait_semaphores;
    };

    class RenderWindow
    {
    public:
        RenderWindow(RenderDevice* device, Window* window, bool vsync, CommandAllocator* command_allocator);

        void present(const PresentDesc& desc);

    private:
        struct FrameData {
            std::unique_ptr<CommandList> command_list;
            DescriptorHandle descriptor;
            UniqueFence fence;
            UniqueSemaphore semaphore;
        };

        [[nodiscard]] static SwapchainDesc swapchain_desc(const Vector2_u& size, bool vsync) noexcept;

        [[nodiscard]] UniqueRenderPass create_render_pass() const;
        [[nodiscard]] std::vector<UniqueImageView> create_image_views() const;
        [[nodiscard]] std::vector<UniqueFramebuffer> create_framebuffers() const;
        [[nodiscard]] UniqueDescriptorLayout create_descriptor_layout() const;
        [[nodiscard]] UniquePipelineLayout create_pipeline_layout() const;
        [[nodiscard]] UniqueDescriptorPool create_descriptor_pool() const;
        [[nodiscard]] UniquePipeline create_pipeline() const;
        [[nodiscard]] UniqueSampler create_sampler() const;
        [[nodiscard]] FrameData create_frame_data(CommandAllocator* command_allocator) const;

        RenderDevice* device_;
        Window* window_;
        std::unique_ptr<Swapchain> swapchain_;
        UniqueRenderPass present_pass_;
        std::vector<UniqueImageView> image_views_;
        std::vector<UniqueFramebuffer> framebuffers_;
        UniqueDescriptorLayout descriptor_layout_;
        UniquePipelineLayout pipeline_layout_;
        UniqueDescriptorPool descriptor_pool_;
        UniquePipeline pipeline_;
        UniqueSampler sampler_;
        PerFrameData<FrameData> frames_;
    };
} // namespace orion
