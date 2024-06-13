#include "orion-renderapi/swapchain.h"

namespace orion
{
    void Swapchain::present(std::uint32_t sync_interval)
    {
        present_api(sync_interval);
    }
} // namespace orion
