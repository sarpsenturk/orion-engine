#include "orion-renderapi/defs.h"

#include "orion-utils/assertion.h"
#include "orion-utils/hash.h"

#include <bit>
#include <numeric>

namespace orion
{
    std::uint32_t vertex_input_stride(std::span<const VertexAttributeDesc> attributes)
    {
        return std::accumulate(attributes.begin(), attributes.end(), 0u, [](auto acc, const auto& attr) {
            return acc + format_size(attr.format);
        });
    }

    std::size_t DescriptorBindingDesc::hash() const
    {
        return std::bit_cast<std::size_t>(*this);
    }

    std::size_t DescriptorLayoutDesc::hash() const
    {
        return std::accumulate(bindings.begin(), bindings.end(), 0ull, [](auto acc, const auto& binding) {
            return hash_combine(acc, binding.hash());
        });
    }

    const char* format_as(PhysicalDeviceType type) noexcept
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

    const char* format_as(Format format) noexcept
    {
        switch (format) {
            case Format::Undefined:
                return "Undefined";
            case Format::B8G8R8A8_Srgb:
                return "B8G8R8A8_Srgb";
            case Format::R8_Unorm:
                return "R8_Unorm";
            case Format::R32_Uint:
                return "R32_Uint";
            case Format::R32_Int:
                return "R32_Int";
            case Format::R32_Float:
                return "R32_Float";
            case Format::R32G32_Uint:
                return "R32G32_Uint";
            case Format::R32G32_Int:
                return "R32G32_Int";
            case Format::R32G32_Float:
                return "R32G32_Float";
            case Format::R32G32B32_Uint:
                return "R32G32B32_Uint";
            case Format::R32G32B32_Int:
                return "R32G32B32_Int";
            case Format::R32G32B32_Float:
                return "R32G32B32_Float";
            case Format::R32G32B32A32_Uint:
                return "R32G32B32A32_Uint";
            case Format::R32G32B32A32_Int:
                return "R32G32B32A32_Int";
            case Format::R32G32B32A32_Float:
                return "R32G32B32A32_Float";
            case Format::R8G8B8A8_Unorm:
                return "R8G8B8A8_Unorm";
        }
        return "Unknown format";
    }
} // namespace orion
