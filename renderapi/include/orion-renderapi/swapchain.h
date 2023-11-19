#pragma once

#include "handles.h"
#include "orion-renderapi/defs.h"

#include <cstdint>

namespace orion
{
    class Swapchain
    {
    public:
        Swapchain() = default;
        virtual ~Swapchain() = default;

        std::uint32_t current_image_index();
        ImageHandle get_image(std::uint32_t image_index);
        void resize_images(std::uint32_t image_count, Format image_format, const Vector2_u& image_size, ImageUsageFlags image_usage);
        void present();

    protected:
        Swapchain(const Swapchain&) = default;
        Swapchain(Swapchain&&) noexcept = default;
        Swapchain& operator=(const Swapchain&) = default;
        Swapchain& operator=(Swapchain&&) noexcept = default;

    private:
        virtual std::uint32_t current_image_index_api() = 0;
        virtual ImageHandle get_image_api(std::uint32_t image_index) = 0;
        virtual void resize_images_api(std::uint32_t image_count, Format image_format, const Vector2_u& image_size, ImageUsageFlags image_usage) = 0;
        virtual void present_api() = 0;
    };
} // namespace orion
