#include "orion/renderapi/format.h"

#include "orion/assertion.h"

#include <cstdint>

namespace orion
{
    std::size_t format_byte_size(Format format)
    {
        switch (format) {
            case Format::Undefined:
                return 0;
            case Format::B8G8R8A8_Unorm:
                return sizeof(uint8_t) * 4;
            case Format::R32G32B32_Float:
                return sizeof(float) * 3;
            case Format::R32G32B32A32_Float:
                return sizeof(float) * 4;
        }
        unreachable();
    }
} // namespace orion
