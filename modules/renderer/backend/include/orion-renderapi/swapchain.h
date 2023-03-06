#pragma once

#include "handles.h"
#include "types.h"

namespace orion
{
    class Swapchain
    {
    public:
        Swapchain() = default;
        Swapchain(SwapchainHandleRef handle, SwapchainDesc desc);

        [[nodiscard]] auto& handle() const noexcept { return handle_; }

        [[nodiscard]] auto image_count() const noexcept { return desc_.image_count; }
        [[nodiscard]] auto image_format() const noexcept { return desc_.image_format; }
        [[nodiscard]] auto image_size() const noexcept { return desc_.image_size; }
        [[nodiscard]] auto& desc() const noexcept { return desc_; }

    private:
        SwapchainHandleRef handle_;

        SwapchainDesc desc_;
    };
} // namespace orion
