#include "orion-renderapi/pipeline.h"

namespace orion
{
    VertexBinding::VertexBinding(std::initializer_list<VertexAttributeDesc> attributes, InputRate input_rate)
        : attributes_(attributes)
        , input_rate_(input_rate)
        , stride_(calculate_stride_and_offsets())
    {
    }

    std::uint32_t VertexBinding::calculate_stride_and_offsets() noexcept
    {
        std::uint32_t offset = 0;
        for (auto& attribute : attributes_) {
            attribute.offset = offset;
            offset += size_of(attribute.format);
        }
        return offset;
    }
} // namespace orion
