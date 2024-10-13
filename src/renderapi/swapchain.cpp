#include "orion/renderapi/swapchain.hpp"

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

    void Swapchain::present(bool vsync)
    {
        present_api(vsync);
    }
} // namespace orion
