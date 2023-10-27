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

    void Swapchain::resize_images(std::uint32_t image_count, Format image_format, const Vector2_u& image_size, ImageUsageFlags image_usage)
    {
        return resize_images_api(image_count, image_format, image_size, image_usage);
    }

    void Swapchain::present()
    {
        present_api();
    }
} // namespace orion
