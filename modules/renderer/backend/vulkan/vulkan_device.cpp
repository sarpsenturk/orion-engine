#include "vulkan_device.h"

#include "vulkan_conversion.h"
#include "vulkan_platform.h"
#include "vulkan_render_context.h"

#include <algorithm>               // std::ranges::transform
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

        std::vector<UniqueVkImageView> create_image_views(VkDevice device, const std::vector<VkImage>& images, VkImageViewType image_view_type, VkFormat format)
        {
            std::vector<UniqueVkImageView> image_views;
            image_views.reserve(images.size());
            for (auto image : images) {
                const VkImageViewCreateInfo info{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .image = image,
                    .viewType = image_view_type,
                    .format = format,
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
                VkImageView image_view = VK_NULL_HANDLE;
                vk_result_check(vkCreateImageView(device, &info, alloc_callbacks(), &image_view));
                SPDLOG_LOGGER_DEBUG(logger_raw(), "Created VkImageView {}", fmt::ptr(image_view));
                image_views.emplace_back(image_view, ImageViewDeleter{device});
            }
            return image_views;
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

        // Create the swapchain
        const VkFormat vk_format = to_vulkan_type(desc.image_format);
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
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        vk_result_check(vkCreateSwapchainKHR(*device_, &info, alloc_callbacks(), &swapchain));
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Created VkSwapchainKHR {}", fmt::ptr(swapchain));

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

        VkRenderPass render_pass = VK_NULL_HANDLE;
        vk_result_check(vkCreateRenderPass(*device_, &render_pass_info, alloc_callbacks(), &render_pass));
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Created VkRenderPass {} for swapchain", fmt::ptr(render_pass));

        auto handle = existing.is_valid() ? existing : SwapchainHandle::generate();
        swapchains_.insert_or_assign(handle, VulkanSwapchain{
                                                 std::move(surface),
                                                 UniqueVkSwapchainKHR{swapchain, SwapchainDeleter{*device_}},
                                                 create_image_views(*device_, get_swapchain_images(*device_, swapchain), VK_IMAGE_VIEW_TYPE_2D, vk_format),
                                                 UniqueVkRenderPass{render_pass, RenderPassDeleter{*device_}},
                                             });
        return handle;
    }

    RenderPassHandle VulkanDevice::create_render_pass_api(const RenderPassDesc& desc, RenderPassHandle existing)
    {
        const auto num_input_attachments = desc.input_attachments.size();
        const auto num_color_attachments = desc.color_attachments.size();
        const auto num_attachments = num_input_attachments + num_color_attachments;

        std::vector<VkAttachmentDescription> vk_attachments;
        vk_attachments.reserve(num_attachments);

        // Helper lambda to convert our AttachmentDesc structs to VkAttachmentDescription and VkAttachmentReference
        auto convert_attachments = [&vk_attachments](std::span<const AttachmentDesc> attachments) {
            std::vector<VkAttachmentReference> attachment_refs;
            attachment_refs.reserve(attachments.size());
            for (const auto& attachment : attachments) {
                const auto attachment_index = vk_attachments.size() - 1;
                vk_attachments.push_back(VkAttachmentDescription{
                    .flags = 0,
                    .format = to_vulkan_type(attachment.format),
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = to_vulkan_type(attachment.load_op),
                    .storeOp = to_vulkan_type(attachment.store_op),
                    .stencilLoadOp = to_vulkan_type(attachment.stencil_load_op),
                    .stencilStoreOp = to_vulkan_type(attachment.stencil_store_op),
                    .initialLayout = to_vulkan_type(attachment.initial_layout),
                    .finalLayout = to_vulkan_type(attachment.final_layout),
                });
                attachment_refs.push_back(VkAttachmentReference{
                    .attachment = static_cast<std::uint32_t>(attachment_index),
                    .layout = to_vulkan_type(attachment.layout),
                });
            }
            return attachment_refs;
        };

        const auto input_attachments = convert_attachments(desc.input_attachments);
        const auto color_attachments = convert_attachments(desc.color_attachments);

        // We support a single subpass as DX12 does not support subpasses
        const VkSubpassDescription subpass_description{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = static_cast<std::uint32_t>(input_attachments.size()),
            .pInputAttachments = input_attachments.data(),
            .colorAttachmentCount = static_cast<std::uint32_t>(color_attachments.size()),
            .pColorAttachments = color_attachments.data(),
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
        };

        const VkRenderPassCreateInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = static_cast<std::uint32_t>(vk_attachments.size()),
            .pAttachments = vk_attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass_description,
            .dependencyCount = 0,
            .pDependencies = nullptr,
        };

        VkRenderPass render_pass = VK_NULL_HANDLE;
        vk_result_check(vkCreateRenderPass(*device_, &render_pass_info, alloc_callbacks(), &render_pass));
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Created VkRenderPass {}", fmt::ptr(render_pass));

        auto handle = existing.is_valid() ? existing : RenderPassHandle::generate();
        render_passes_.insert_or_assign(handle, UniqueVkRenderPass{render_pass, RenderPassDeleter{*device_}});
        return handle;
    }

    void VulkanDevice::destroy(SwapchainHandle swapchain_handle)
    {
        swapchains_.erase(swapchain_handle);
    }

    void VulkanDevice::destroy(RenderPassHandle render_pass_handle)
    {
        render_passes_.erase(render_pass_handle);
    }
} // namespace orion::vulkan