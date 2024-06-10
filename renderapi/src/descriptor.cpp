#include "orion-renderapi/descriptor.h"

#include "orion-utils/hash.h"

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
} // namespace orion
