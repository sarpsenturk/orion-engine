#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include <vector> // std::vector

namespace orion::vulkan
{
    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(UniqueVkSurfaceKHR surface,
                        UniqueVkSwapchainKHR swapchain,
                        UniqueVkRenderPass render_pass,
                        UniqueVkSemaphore image_available,
                        std::vector<UniqueVkImageView> image_views,
                        std::vector<UniqueVkFramebuffer> framebuffers);

        [[nodiscard]] auto surface() const noexcept { return surface_.get(); }
        [[nodiscard]] auto swapchain() const noexcept { return swapchain_.get(); }
        [[nodiscard]] auto render_pass() const noexcept { return render_pass_.get(); }
        [[nodiscard]] auto image_semaphore() const noexcept { return image_available_.get(); }
        [[nodiscard]] auto framebuffer(std::uint32_t index) const noexcept { return framebuffers_[index].get(); }
        [[nodiscard]] std::uint32_t current_image_index() const { return available_image_; }
        [[nodiscard]] std::uint32_t next_image_index();

    private:
        UniqueVkSurfaceKHR surface_;
        UniqueVkSwapchainKHR swapchain_;
        UniqueVkRenderPass render_pass_;
        UniqueVkSemaphore image_available_;
        std::vector<UniqueVkImageView> image_views_;
        std::vector<UniqueVkFramebuffer> framebuffers_;
        std::uint32_t available_image_ = UINT32_MAX;
    };
} // namespace orion::vulkan
