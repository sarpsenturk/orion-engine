#include "vulkan_device.h"

#include "vulkan_conversion.h"
#include "vulkan_platform.h"
#include "vulkan_render_context.h"

#include <algorithm>               // std::ranges::transform, std::ranges::for_each
#include <orion-utils/assertion.h> // ORION_EXPECTS
#include <spdlog/spdlog.h>         // SPDLOG_*
#include <utility>                 // std::exchange

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

    VulkanDevice::VulkanDevice(VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues)
        : instance_(instance)
        , physical_device_(physical_device)
        , device_(std::move(device))
        , queues_(queues)
    {
    }

    std::unique_ptr<RenderContext> VulkanDevice::create_render_context()
    {
        const VkCommandPoolCreateInfo command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queues_.graphics.index,
        };
        VkCommandPool command_pool = VK_NULL_HANDLE;
        vk_result_check(vkCreateCommandPool(*device_, &command_pool_create_info, alloc_callbacks(), &command_pool));
        return std::make_unique<VulkanRenderContext>(UniqueVkCommandPool{command_pool, CommandPoolDeleter{*device_}});
    }

    SwapchainHandle VulkanDevice::create_swapchain_api(const Window& window, const SwapchainDesc& desc, SwapchainHandle existing)
    {
        // Create the surface
        auto surface = create_surface(instance_, window);

        // Chose present mode TODO: Allow user to select this
        const auto present_mode = VK_PRESENT_MODE_FIFO_KHR;

        // Get the surface capabilities
        const auto surface_capabilities = [this, &surface]() {
            VkSurfaceCapabilitiesKHR surface_capabilities;
            vk_result_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, *surface, &surface_capabilities));
            return surface_capabilities;
        }();

        // Get the old swapchain
        VkSwapchainKHR old_swapchain = [this, existing]() -> VkSwapchainKHR {
            if (existing.is_valid()) {
                return swapchains_.at(existing).swapchain();
            }
            return VK_NULL_HANDLE;
        }();

        const VkFormat vk_format = to_vulkan_type(desc.image_format);

        // Create the swapchain
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        {
            const VkSwapchainCreateInfoKHR info{
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = *surface,
                .minImageCount = desc.image_count,
                .imageFormat = vk_format,
                .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
                .imageExtent = to_vulkan_extent(desc.image_size),
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .preTransform = surface_capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = present_mode,
                .clipped = VK_TRUE,
                .oldSwapchain = old_swapchain,
            };
            vk_result_check(vkCreateSwapchainKHR(*device_, &info, alloc_callbacks(), &swapchain));
            SPDLOG_LOGGER_DEBUG(logger_raw(), "Created VkSwapchainKHR {}", fmt::ptr(swapchain));
        }

        // Create render pass
        VkRenderPass render_pass = VK_NULL_HANDLE;
        {
            const VkAttachmentDescription color_attachment{
                .flags = 0,
                .format = vk_format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            };

            const VkAttachmentReference color_attachment_ref{
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };

            const VkSubpassDescription subpass{
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = 0,
                .pInputAttachments = nullptr,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color_attachment_ref,
                .pResolveAttachments = nullptr,
                .pDepthStencilAttachment = nullptr,
                .preserveAttachmentCount = 0,
                .pPreserveAttachments = nullptr,
            };

            const VkRenderPassCreateInfo render_pass_info{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .attachmentCount = 1,
                .pAttachments = &color_attachment,
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 0,
                .pDependencies = nullptr,
            };

            vk_result_check(vkCreateRenderPass(*device_, &render_pass_info, alloc_callbacks(), &render_pass));
            SPDLOG_LOGGER_DEBUG(logger_raw(), "Created render pass for swapchain", fmt::ptr(render_pass));
        }

        // Create image view for each image and framebuffer for each image view
        std::vector<UniqueVkImageView> image_views;
        std::vector<UniqueVkFramebuffer> framebuffers;
        {
            // Acquire swapchain images
            auto swapchain_images = get_swapchain_images(*device_, swapchain);

            image_views.reserve(swapchain_images.size());
            framebuffers.reserve(swapchain_images.size());
            for (auto image : swapchain_images) {
                // Create image view
                VkImageView image_view = VK_NULL_HANDLE;
                {
                    const VkImageViewCreateInfo image_view_info{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = 0,
                        .image = image,
                        .viewType = VK_IMAGE_VIEW_TYPE_2D,
                        .format = vk_format,
                        .components = {
                            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                        },
                        .subresourceRange = {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        },
                    };
                    vk_result_check(vkCreateImageView(*device_, &image_view_info, alloc_callbacks(), &image_view));
                    image_views.emplace_back(image_view, ImageViewDeleter{*device_});
                }

                // Create framebuffer
                {
                    const VkFramebufferCreateInfo framebuffer_info{
                        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = 0,
                        .renderPass = render_pass,
                        .attachmentCount = 1,
                        .pAttachments = &image_view,
                        .width = desc.image_size.x(),
                        .height = desc.image_size.y(),
                        .layers = 1,
                    };
                    VkFramebuffer framebuffer = VK_NULL_HANDLE;
                    vk_result_check(vkCreateFramebuffer(*device_, &framebuffer_info, alloc_callbacks(), &framebuffer));
                    framebuffers.emplace_back(framebuffer, FramebufferDeleter{*device_});
                }
            }
            SPDLOG_LOGGER_DEBUG(logger_raw(), "Created {} image view(s) and framebuffer(s) for swapchain", swapchain_images.size());
        }

        auto handle = existing.is_valid() ? existing : SwapchainHandle::generate();
        swapchains_.insert_or_assign(handle, VulkanSwapchain{
                                                 std::move(surface),
                                                 UniqueVkSwapchainKHR{swapchain, SwapchainDeleter{*device_}},
                                                 UniqueVkRenderPass{render_pass, RenderPassDeleter{*device_}},
                                                 std::move(image_views),
                                                 std::move(framebuffers),
                                             });
        return handle;
    }

    void VulkanDevice::destroy(SwapchainHandle swapchain_handle)
    {
        swapchains_.erase(swapchain_handle);
    }
} // namespace orion::vulkan
