#pragma once

#include <cstdint>            // std::uint32_t
#include <orion-utils/enum.h> // orion::enum_traits
#include <string>             // std::string

namespace orion
{
    enum class PhysicalDeviceType {
        Other,
        Integrated,
        Discrete,
        Virtual,
        CPU
    };

    template<>
    struct enum_traits<PhysicalDeviceType> : default_enum_traits<PhysicalDeviceType> {
        static constexpr auto to_string(PhysicalDeviceType type) noexcept
        {
            switch (type) {
                case PhysicalDeviceType::Other:
                    break;
                case PhysicalDeviceType::Integrated:
                    return "Integrated";
                case PhysicalDeviceType::Discrete:
                    return "Discrete";
                case PhysicalDeviceType::Virtual:
                    return "Virtual";
                case PhysicalDeviceType::CPU:
                    return "CPU";
            }
            return "Other";
        }
    };

    struct PhysicalDeviceDesc {
        std::uint32_t index;
        PhysicalDeviceType type;
        std::string name;
    };
} // namespace orion
