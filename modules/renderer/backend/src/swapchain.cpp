#include "orion-renderapi/swapchain.h"

namespace orion
{
    Swapchain::Swapchain(SwapchainHandle handle, SwapchainDesc desc)
        : handle_(handle)
        , desc_(desc)
    {
    }
} // namespace orion
