#include "orion-renderapi/descriptor.h"

#include "orion-utils/functors.h"

#include <numeric>

namespace orion
{
    std::size_t DescriptorBinding::hash() const
    {
        return 0ull |
               to_underlying(type) |
               std::size_t{shader_stages.value()} << 8ull |
               std::size_t{count} << 16ull;
    }

    DescriptorSetLayout::DescriptorSetLayout(std::initializer_list<DescriptorBinding> bindings)
        : bindings_(bindings)
        , hash_(hash_bindings(bindings))
    {
    }

    std::size_t DescriptorSetLayout::hash_bindings(std::span<const DescriptorBinding> bindings)
    {
        return std::accumulate(bindings.begin(), bindings.end(), bindings.size(), [](auto lhs, const auto& rhs) {
            return lhs ^ rhs.hash();
        });
    }
} // namespace orion
