#pragma once

#include <cstdint>
#include <limits>

namespace orion
{
    using render_device_handle_t = std::uint64_t;
    inline constexpr render_device_handle_t invalid_device_handle = std::numeric_limits<render_device_handle_t>::max();

    enum class PipelineLayoutHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class PipelineHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class BufferHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class ImageHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class RenderTargetHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class SemaphoreHandle : render_device_handle_t { Invalid = invalid_device_handle };
    enum class FenceHandle : render_device_handle_t { Invalid = invalid_device_handle };
} // namespace orion
