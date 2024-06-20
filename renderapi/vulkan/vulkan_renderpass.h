#pragma once

#include "orion-renderapi/render_pass.h"

#include "vulkan_headers.h"

#include <vector>

namespace orion::vulkan
{
    class VulkanResourceManager;

    class VulkanRenderPass final : public RenderPass
    {
    public:
        explicit VulkanRenderPass(VulkanResourceManager* resource_manager);

        [[nodiscard]] auto& color_attachments() const { return color_attachments_; }

    private:
        render_pass_attachment_t add_attachment_api() override;
        void clear_api(render_pass_attachment_t attachment, const Vector4_f& clear_color) override;
        void set_render_target_api(render_pass_attachment_t attachment, ImageViewHandle render_target) override;

        VulkanResourceManager* resource_manager_;
        std::vector<VkRenderingAttachmentInfoKHR> color_attachments_;
    };
} // namespace orion::vulkan
