#include "orion_dx12.h"

#include <fmt/format.h>

#include <stdexcept>

namespace orion
{
    void hr_assert(HRESULT hr, const char* msg)
    {
        if (FAILED(hr)) [[unlikely]] {
            throw std::runtime_error{fmt::format("{}: {}", msg, hr)};
        }
    }
} // namespace orion
