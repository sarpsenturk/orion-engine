#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include <vector> // std::vector

namespace orion::vulkan
{
    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(VkDevice device, VkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain, VkFormat format, const math::Vector2_u& size);

        [[nodiscard]] auto surface() const noexcept { return surface_; }
        [[nodiscard]] auto swapchain() const noexcept { return swapchain_.get(); }
        [[nodiscard]] auto render_pass() const noexcept { return render_pass_.get(); }
        [[nodiscard]] auto image_semaphore() const noexcept { return image_semaphore_.get(); }
        [[nodiscard]] auto framebuffer(std::uint32_t index) const noexcept { return framebuffers_[index].get(); }
        [[nodiscard]] std::uint32_t current_image_index() const { return available_image_; }
        [[nodiscard]] std::uint32_t next_image_index();

    private:
        static UniqueVkRenderPass create_render_pass_for_swapchain(VkDevice device, VkFormat format);
        static std::vector<UniqueVkImageView> create_image_views_for_swapchain(VkDevice device, VkSwapchainKHR swapchain, VkFormat format);
        static std::vector<UniqueVkFramebuffer> create_framebuffers_for_swapchain(VkDevice device,
                                                                                  VkRenderPass render_pass,
                                                                                  const math::Vector2_u& size,
                                                                                  const std::vector<UniqueVkImageView>& image_views);

        VkSurfaceKHR surface_;
        UniqueVkSwapchainKHR swapchain_;
        UniqueVkRenderPass render_pass_;
        UniqueVkSemaphore image_semaphore_;
        std::vector<UniqueVkImageView> image_views_;
        std::vector<UniqueVkFramebuffer> framebuffers_;
        std::uint32_t available_image_ = UINT32_MAX;
    };
} // namespace orion::vulkan
