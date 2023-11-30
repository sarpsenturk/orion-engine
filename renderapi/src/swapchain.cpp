#include "orion-renderapi/swapchain.h"

namespace orion
{
    std::uint32_t Swapchain::current_image_index()
    {
        return current_image_index_api();
    }

    ImageHandle Swapchain::get_image(std::uint32_t image_index)
    {
        return get_image_api(image_index);
    }

    void Swapchain::resize_images(const SwapchainDesc& desc)
    {
        return resize_images_api(desc);
    }

    void Swapchain::present(std::span<const SemaphoreHandle> wait_semaphores)
    {
        present_api(wait_semaphores);
    }
} // namespace orion
