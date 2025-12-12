#include "orion/rhi/command.hpp"

namespace orion
{
    bool RHICommandAllocator::reset()
    {
        return reset_api();
    }

    bool RHICommandList::reset()
    {
        return reset_api();
    }

    void RHICommandList::begin()
    {
        return begin_api();
    }

    void RHICommandList::end()
    {
        return end_api();
    }
} // namespace orion
