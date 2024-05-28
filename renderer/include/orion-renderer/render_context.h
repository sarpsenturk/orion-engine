#pragma once

#include "orion-renderer/config.h"
#include "orion-renderer/render_target.h"

#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_command.h"

#include "orion-utils/static_vector.h"

#include "orion-math/vector/vector2.h"

#include <cstddef>
#include <memory>
#include <span>

namespace orion
{
    struct FrameInFlight {
        std::unique_ptr<CommandAllocator> command_allocator;

        std::unique_ptr<CommandList> render_command;
        FenceHandle render_fence;
        SemaphoreHandle render_semaphore;
        RenderTarget render_target;

        DescriptorHandle render_output_descriptor;

        std::unique_ptr<CommandList> present_command;
        FenceHandle present_fence;
        SemaphoreHandle present_semaphore;

        GPUBufferHandle staging_buffer;
    };

    struct FrameInFlightDesc {
        Vector2_u render_size;
        DescriptorLayoutHandle present_descriptor_layout;
        SamplerHandle present_sampler;
    };

    struct CopyBufferStaging {
        std::span<const std::byte> bytes;
        GPUBufferHandle dst;
        std::size_t dst_offset = 0ull;
    };

    // Forward declare
    class RenderDevice;

    class RenderContext
    {
    public:
        RenderContext(RenderDevice* device, const FrameInFlightDesc& desc);

        [[nodiscard]] frame_index_t current_frame_index() const noexcept { return current_frame_index_; }
        [[nodiscard]] frame_index_t previous_frame_index() const noexcept { return previous_frame_index_; }

        [[nodiscard]] auto& current_frame() const { return frames_[current_frame_index_]; }
        [[nodiscard]] auto& previous_frame() const { return frames_[previous_frame_index_]; }

        void advance_frame();

        RenderDevice* device() const noexcept { return device_; }
        CommandAllocator* command_allocator() const noexcept { return current_frame().command_allocator.get(); }
        GPUBufferHandle staging_buffer() const noexcept { return current_frame().staging_buffer; }

        void copy_buffer_staging(const CopyBufferStaging& copy);
        void copy_buffer_staging(std::span<const CopyBufferStaging> copies);

    private:
        RenderDevice* device_;
        frame_index_t current_frame_index_ = 0;
        frame_index_t previous_frame_index_ = -1;
        static_vector<FrameInFlight, frames_in_flight> frames_;
    };
} // namespace orion
