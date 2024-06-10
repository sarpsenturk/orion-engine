#pragma once

#include "orion-renderer/frame.h"
#include "orion-renderer/render_target.h"

#include "orion-renderapi/handles.h"
#include "orion-renderapi/render_command.h"

#include "orion-utils/static_vector.h"

#include "orion-math/matrix/matrix4.h"
#include "orion-math/vector/vector2.h"
#include "orion-math/vector/vector3.h"

#include <cstddef>
#include <memory>
#include <span>

namespace orion
{
    struct CopyBufferStaging {
        std::span<const std::byte> bytes;
        GPUBufferHandle dst;
        std::size_t dst_offset = 0ull;
    };

    struct CopyImageStaging {
        std::span<const std::byte> bytes;
        ImageHandle dst;
        ImageLayout dst_initial_layout;
        ImageLayout dst_final_layout;
        std::size_t dst_offset = 0ull;
        Vector3_u dst_size;
    };

    // Forward declare
    class RenderDevice;

    class TransferContext
    {
    public:
        TransferContext(RenderDevice* device, CommandAllocator* command_allocator, GPUBufferHandle staging_buffer);

        void copy_buffer_staging(const CopyBufferStaging& copy);
        void copy_buffer_staging(std::span<const CopyBufferStaging> copies);

        void copy_image_staging(const CopyImageStaging& copy);

        void memcpy(ImageHandle image, std::span<const std::byte> bytes, std::size_t offset = 0ull);

    private:
        RenderDevice* device_;
        CommandAllocator* command_allocator_;
        GPUBufferHandle staging_buffer_;
    };
} // namespace orion
