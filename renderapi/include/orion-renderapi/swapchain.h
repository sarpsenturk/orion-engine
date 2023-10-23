#pragma once

#include "handles.h"

namespace orion
{
    class Swapchain
    {
    public:
        Swapchain() = default;
        virtual ~Swapchain() = default;

    protected:
        Swapchain(const Swapchain&) = default;
        Swapchain(Swapchain&&) noexcept = default;
        Swapchain& operator=(const Swapchain&) = default;
        Swapchain& operator=(Swapchain&&) noexcept = default;

    private:
    };
} // namespace orion
