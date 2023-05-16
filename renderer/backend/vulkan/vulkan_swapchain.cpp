#include "vulkan_swapchain.h"

#include <array> // std::array

namespace orion::vulkan
{
    namespace
    {
        std::vector<VkImage> get_swapchain_images(VkDevice device, VkSwapchainKHR swapchain)
        {
            std::uint32_t image_count = 0;
            vk_result_check(vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr));
            std::vector<VkImage> swapchain_images(image_count);
            vk_result_check(vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data()));
            return swapchain_images;
        }
    } // namespace

    VulkanSwapchain::VulkanSwapchain(VkDevice device, VkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain, VkFormat format, const math::Vector2_u& size)
        : surface_(surface)
        , swapchain_(std::move(swapchain))
        , render_pass_(create_render_pass_for_swapchain(device, format))
        , image_semaphore_(create_vk_semaphore(device))
        , image_views_(create_image_views_for_swapchain(device, swapchain_.get(), format))
        , framebuffers_(create_framebuffers_for_swapchain(device, render_pass_.get(), size, image_views_))
    {
    }

    UniqueVkRenderPass VulkanSwapchain::create_render_pass_for_swapchain(VkDevice device, VkFormat format)
    {
        const std::array attachments{
            VkAttachmentDescription{
                .flags = 0,
                .format = format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            },
        };
        const std::array color_attachments{
            VkAttachmentReference{
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
        };
        const std::array subpass_dependencies{
            VkSubpassDependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dependencyFlags = 0,
            },
        };
        return create_vk_render_pass(device, VK_PIPELINE_BIND_POINT_GRAPHICS, attachments, color_attachments, subpass_dependencies);
    }

    std::vector<UniqueVkImageView> VulkanSwapchain::create_image_views_for_swapchain(VkDevice device, VkSwapchainKHR swapchain, VkFormat format)
    {
        // Acquire swapchain images
        auto swapchain_images = get_swapchain_images(device, swapchain);

        // Create image views
        std::vector<UniqueVkImageView> image_views;
        image_views.reserve(swapchain_images.size());
        for (VkImage image : swapchain_images) {
            image_views.push_back(create_vk_image_view(device, image, VK_IMAGE_VIEW_TYPE_2D, format));
        }
        return image_views;
    }

    std::vector<UniqueVkFramebuffer> VulkanSwapchain::create_framebuffers_for_swapchain(VkDevice device,
                                                                                        VkRenderPass render_pass,
                                                                                        const math::Vector2_u& size,
                                                                                        const std::vector<UniqueVkImageView>& image_views)
    {
        std::vector<UniqueVkFramebuffer> framebuffers;
        framebuffers.reserve(image_views.size());
        for (const auto& image_view : image_views) {
            VkImageView vk_image_view = image_view.get();
            framebuffers.push_back(create_vk_framebuffer(device, render_pass, size, {&vk_image_view, 1}));
        }
        return framebuffers;
    }

    std::uint32_t VulkanSwapchain::next_image_index()
    {
        VkDevice device = swapchain_.get_deleter().device;
        vk_result_check(vkAcquireNextImageKHR(device, swapchain(), UINT64_MAX, image_semaphore(), VK_NULL_HANDLE, &available_image_));
        return available_image_;
    }
} // namespace orion::vulkan
