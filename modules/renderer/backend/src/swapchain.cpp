#include "orion-renderapi/swapchain.h"

namespace orion
{
    Swapchain::Swapchain(SwapchainHandleRef handle, SwapchainDesc desc)
        : handle_(std::move(handle))
        , desc_(desc)
    {
    }
} // namespace orion
