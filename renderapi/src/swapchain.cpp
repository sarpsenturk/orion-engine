#include "orion-renderapi/swapchain.h"

namespace orion
{
    std::uint32_t Swapchain::current_image_index()
    {
        return current_image_index_api();
    }

    void Swapchain::present()
    {
        present_api();
    }
} // namespace orion
