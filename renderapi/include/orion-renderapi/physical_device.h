#pragma once

#include <cstdint>
#include <string>

namespace orion
{
    enum class PhysicalDeviceType {
        Other,
        Integrated,
        Discrete,
        Virtual,
        CPU
    };

    const char* format_as(PhysicalDeviceType type) noexcept;

    using physical_device_index_t = std::int32_t;
    inline constexpr auto invalid_physical_device_index = -1;

    struct PhysicalDeviceDesc {
        physical_device_index_t index;
        PhysicalDeviceType type;
        std::string name;
    };

    constexpr bool is_discrete_gpu(const PhysicalDeviceDesc& device) noexcept
    {
        return device.type == PhysicalDeviceType::Discrete;
    }
} // namespace orion
