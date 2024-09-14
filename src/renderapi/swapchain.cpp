#include "orion/renderapi/swapchain.h"

namespace orion
{
    RenderTargetHandle Swapchain::acquire_render_target()
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
