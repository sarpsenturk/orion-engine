#pragma once

#include <cstdint>
#include <limits>

namespace orion
{
    using render_device_handle_t = std::uint64_t;
    inline constexpr render_device_handle_t invalid_device_handle = std::numeric_limits<std::uint64_t>::max();

    enum class GraphicsPipelineHandle : render_device_handle_t {};
} // namespace orion
