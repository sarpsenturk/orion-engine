#include "vulkan_renderpass.h"

#include "vulkan_conversion.h"
#include "vulkan_resource.h"

#include "orion-utils/assertion.h"

namespace orion::vulkan
{
    VulkanRenderPass::VulkanRenderPass(VulkanResourceManager* resource_manager)
        : resource_manager_(resource_manager)
    {
    }

    render_pass_attachment_t VulkanRenderPass::add_attachment_api()
    {
        const auto attachment = static_cast<render_pass_attachment_t>(color_attachments_.size());
        color_attachments_.push_back({
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .pNext = nullptr,
            .imageView = VK_NULL_HANDLE, // This is set when begin render pass is called
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE_KHR,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, // This can be set to VK_ATTACHMENT_LOAD_OP_CLEAR by calling RenderPass::clear()
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {}, // This is set when RenderPass::clear() is called
        });
        return attachment;
    }

    void VulkanRenderPass::clear_api(render_pass_attachment_t attachment, const Vector4_f& clear_color)
    {
        ORION_EXPECTS(attachment < color_attachments_.size());
        color_attachments_[attachment].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachments_[attachment].clearValue = {to_vulkan_clear_color(clear_color)};
    }

    void VulkanRenderPass::set_render_target_api(render_pass_attachment_t attachment, ImageViewHandle render_target)
    {
        ORION_EXPECTS(attachment < color_attachments_.size());
        VkImageView vk_image_view = resource_manager_->find(render_target);
        ORION_ASSERT(vk_image_view != VK_NULL_HANDLE);
        color_attachments_[attachment].imageView = vk_image_view;
    }
} // namespace orion::vulkan
