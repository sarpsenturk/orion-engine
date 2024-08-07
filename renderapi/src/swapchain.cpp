#include "orion-renderapi/swapchain.h"

namespace orion
{
    ImageViewHandle Swapchain::acquire_render_target()
    {
        return acquire_render_target_api();
    }

    ImageHandle Swapchain::current_image()
    {
        return current_image_api();
    }

    void Swapchain::present(std::uint32_t sync_interval)
    {
        present_api(sync_interval);
    }
} // namespace orion
