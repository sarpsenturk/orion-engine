#pragma once

#include <cstdint>
#include <limits>

namespace orion
{
    using render_device_handle_t = std::uint32_t;
    inline constexpr render_device_handle_t invalid_device_handle = std::numeric_limits<render_device_handle_t>::max();

    enum class DescriptorLayoutHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class PipelineLayoutHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class PipelineHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class BufferHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class ImageHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class ImageViewHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class SemaphoreHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class FenceHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class DescriptorHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class SamplerHandle : render_device_handle_t { Invalid = invalid_device_handle };
} // namespace orion
