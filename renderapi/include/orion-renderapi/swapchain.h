#pragma once

#include "handles.h"

#include <cstdint>

namespace orion
{
    class Swapchain
    {
    public:
        Swapchain() = default;
        virtual ~Swapchain() = default;

        std::uint32_t current_image_index();
        void present();

    protected:
        Swapchain(const Swapchain&) = default;
        Swapchain(Swapchain&&) noexcept = default;
        Swapchain& operator=(const Swapchain&) = default;
        Swapchain& operator=(Swapchain&&) noexcept = default;

    private:
        virtual std::uint32_t current_image_index_api() = 0;
        virtual void present_api() = 0;
    };
} // namespace orion
