#include "orion-renderapi/types.h"

#include "orion-utils/hash.h"

#include <numeric>

template<>
struct std::hash<orion::DescriptorBinding> {
    std::size_t operator()(const orion::DescriptorBinding& binding) const
    {
        return 0ull |
               to_underlying(binding.type) |
               std::size_t{binding.shader_stages.value()} << 8ull |
               std::size_t{binding.count} << 16ull;
    }
};

namespace orion
{
    DescriptorSetLayout::DescriptorSetLayout(std::initializer_list<DescriptorBinding> bindings)
        : bindings_(bindings)
        , hash_(hash_bindings(bindings))
    {
    }

    std::size_t DescriptorSetLayout::hash_bindings(std::span<const DescriptorBinding> bindings)
    {
        return std::accumulate(bindings.begin(), bindings.end(), bindings.size(), hash_combine<DescriptorBinding>);
    }
} // namespace orion
