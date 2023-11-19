#include "orion-renderapi/types.h"

#include "orion-utils/assertion.h"
#include "orion-utils/hash.h"

#include <bit>
#include <numeric>

namespace orion
{
    std::string orion::format_as(ShaderStageFlags shader_stages)
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

    bool is_buffer_binding(const DescriptorBinding& binding)
    {
        return binding.is_buffer();
    }
} // namespace orion
