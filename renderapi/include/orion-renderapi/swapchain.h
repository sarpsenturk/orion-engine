#pragma once

#include "orion-renderapi/format.h"
#include "orion-renderapi/image.h"

#include "orion-math/vector/vector2.h"

#include <cstdint>
#include <span>

namespace orion
{
    namespace defaults
    {
        inline constexpr auto swapchain_image_count = 2;
        inline constexpr auto swapchain_format = Format::B8G8R8A8_Srgb;
    } // namespace defaults

    struct SwapchainDesc {
        std::uint32_t image_count = defaults::swapchain_image_count;
        Format image_format = defaults::swapchain_format;
        Vector2_u image_size = {};
        ImageUsageFlags image_usage = ImageUsageFlags::ColorAttachment;
        bool vsync = true;
    };

    class Swapchain
    {
    public:
        Swapchain() = default;
        virtual ~Swapchain() = default;

        std::uint32_t current_image_index();
        ImageHandle get_image(std::uint32_t image_index);
        void resize_images(const SwapchainDesc& desc);
        void present(std::span<const SemaphoreHandle> wait_semaphores);

    protected:
        Swapchain(const Swapchain&) = default;
        Swapchain(Swapchain&&) noexcept = default;
        Swapchain& operator=(const Swapchain&) = default;
        Swapchain& operator=(Swapchain&&) noexcept = default;

    private:
        virtual std::uint32_t current_image_index_api() = 0;
        virtual ImageHandle get_image_api(std::uint32_t image_index) = 0;
        virtual void resize_images_api(const SwapchainDesc& desc) = 0;
        virtual void present_api(std::span<const SemaphoreHandle> wait_semaphores) = 0;
    };
} // namespace orion
