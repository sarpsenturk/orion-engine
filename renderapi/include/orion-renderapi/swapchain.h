#pragma once

#include "handles.h"
#include "orion-renderapi/defs.h"

#include <cstdint>
#include <span>

namespace orion
{
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
