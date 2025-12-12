#pragma once

namespace orion
{
    struct RHISemaphoreDesc {
    };

    struct RHIFenceDesc {
        bool create_signaled;
    };
} // namespace orion
