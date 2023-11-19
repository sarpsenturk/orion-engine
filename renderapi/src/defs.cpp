#include "orion-renderapi/defs.h"

#include "orion-utils/assertion.h"
#include "orion-utils/hash.h"

#include <bit>
#include <numeric>

namespace orion
{
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
            case Format::R32G32_Float:
                return "R32G32_Float";
            case Format::R32G32B32_Float:
                return "R32G32B32_Float";
            case Format::R32G32B32A32_Float:
                return "R32G32B32A32_Float";
            case Format::R8_Unorm:
                return "R8_Unorm";
            case Format::R8G8B8A8_Unorm:
                return "R8G8B8A8_Unorm";
        }
        return "Unknown format";
    }

    std::string format_as(ShaderStageFlags shader_stages)
    {
        if (!shader_stages) {
            return {};
        }

        std::string result{};
        for (auto stage : BitwiseRange{shader_stages}) {
            if (!stage) {
                continue;
            }
            switch (stage) {
                case ShaderStageFlags::Vertex:
                    result += "Vertex | ";
                    break;
                case ShaderStageFlags::Pixel:
                    result += "Pixel | ";
                    break;
            }
        }
        return result.substr(0, result.size() - 3);
    }
} // namespace orion
