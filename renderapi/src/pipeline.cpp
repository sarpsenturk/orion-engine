#include "orion-renderapi/pipeline.h"

#include <numeric>

namespace orion
{
    std::uint32_t vertex_input_stride(std::span<const VertexAttributeDesc> attributes)
    {
        return std::accumulate(attributes.begin(), attributes.end(), 0u, [](auto acc, const auto& attr) {
            return acc + format_size(attr.format);
        });
    }
} // namespace orion
