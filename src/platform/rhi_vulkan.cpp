#include "orion/rhi/command.hpp"
#include "orion/rhi/device.hpp"
#include "orion/rhi/format.hpp"
#include "orion/rhi/handle.hpp"
#include "orion/rhi/pipeline.hpp"
#include "orion/rhi/rhi.hpp"
#include "orion/rhi/swapchain.hpp"

#include "orion/assert.hpp"
#include "orion/log.hpp"

#include "orion/utils/finally.hpp"
#include "orion/utils/handle_pool.hpp"

#include <volk.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vk_enum_string_helper.h>

#include "orion/platform/platform.hpp"
#ifdef ORION_OS_MACOS
    #define ORION_VULKAN_MVK 1
#endif

#include "platform_glfw.hpp"
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace orion
{
    namespace
    {
        constexpr auto vulkan_api_version = VK_API_VERSION_1_3;

        struct VulkanQueue {
            std::uint32_t family;
            VkQueue queue;
        };

        struct VulkanSwapchain {
            VkSurfaceKHR surface;
            VkSwapchainKHR swapchain;
            class RHIVulkanCommandQueue* queue;
            std::vector<RHIImage> images;
            std::uint32_t current_image_index = UINT32_MAX;
        };

        struct VulkanPipeline {
            VkPipeline pipeline;
            VkPipelineLayout layout;
        };

        struct VulkanImage {
            VkImage image;
            VkImageLayout current_layout = VK_IMAGE_LAYOUT_UNDEFINED;

            VkImageAspectFlags aspect_flags() const
            {
                if (current_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                } else {
                    return VK_IMAGE_ASPECT_COLOR_BIT;
                }
            }
        };

        struct VulkanImageView {
            VkImageView image_view;
        };

        struct VulkanResourceTable {
            HandlePool<VulkanSwapchain> swapchains;
            HandlePool<VulkanPipeline> pipelines;
            HandlePool<VkSemaphore> semaphores;
            HandlePool<VkFence> fences;
            HandlePool<VulkanImage> images;
            HandlePool<VulkanImageView> image_views;
        };

        VkFormat to_vk_format(RHIFormat format)
        {
            switch (format) {
                case RHIFormat::Unknown:
                    return VK_FORMAT_UNDEFINED;
                case RHIFormat::B8G8R8A8_Unorm_Srgb:
                    return VK_FORMAT_B8G8R8A8_SRGB;
            }
            unreachable();
        }

        std::uint32_t vk_format_size(VkFormat format)
        {
            switch (format) {
                case VK_FORMAT_B8G8R8A8_SRGB:
                    return 4;
                default:
                    ORION_ASSERT(false, "Unhandled format");
            }
            unreachable();
        }

        VkPrimitiveTopology to_vk_primitive_topology(RHIPrimitiveTopology topology)
        {
            switch (topology) {
                case RHIPrimitiveTopology::TriangleList:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }
            unreachable();
        }

        VkPolygonMode to_vk_polygon_mode(RHIFillMode fill_mode)
        {
            switch (fill_mode) {
                case RHIFillMode::Solid:
                    return VK_POLYGON_MODE_FILL;
            }
            unreachable();
        }

        VkCullModeFlags to_vk_cull_mode(RHICullMode cull_mode)
        {
            switch (cull_mode) {
                case RHICullMode::None:
                    return VK_CULL_MODE_NONE;
                case RHICullMode::Front:
                    return VK_CULL_MODE_FRONT_BIT;
                case RHICullMode::Back:
                    return VK_CULL_MODE_BACK_BIT;
            }
            unreachable();
        }

        VkCompareOp to_vk_compare_op(RHICompareOp compare_op)
        {
            switch (compare_op) {
                case RHICompareOp::Never:
                    return VK_COMPARE_OP_NEVER;
                case RHICompareOp::Less:
                    return VK_COMPARE_OP_LESS;
                case RHICompareOp::Equal:
                    return VK_COMPARE_OP_EQUAL;
                case RHICompareOp::LessEqual:
                    return VK_COMPARE_OP_LESS_OR_EQUAL;
                case RHICompareOp::Greater:
                    return VK_COMPARE_OP_GREATER;
                case RHICompareOp::NotEqual:
                    return VK_COMPARE_OP_NOT_EQUAL;
                case RHICompareOp::GreaterEqual:
                    return VK_COMPARE_OP_GREATER_OR_EQUAL;
                case RHICompareOp::Always:
                    return VK_COMPARE_OP_ALWAYS;
            }
            unreachable();
        }

        VkBlendFactor to_vk_blend_factor(RHIBlendFactor blend_factor)
        {
            switch (blend_factor) {
                case RHIBlendFactor::Zero:
                    return VK_BLEND_FACTOR_ZERO;
                case RHIBlendFactor::One:
                    return VK_BLEND_FACTOR_ONE;
                case RHIBlendFactor::SrcColor:
                    return VK_BLEND_FACTOR_SRC_COLOR;
                case RHIBlendFactor::InvSrcColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                case RHIBlendFactor::DstColor:
                    return VK_BLEND_FACTOR_DST_COLOR;
                case RHIBlendFactor::InvDstColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
                case RHIBlendFactor::SrcAlpha:
                    return VK_BLEND_FACTOR_SRC_ALPHA;
                case RHIBlendFactor::InvSrcAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                case RHIBlendFactor::DstAlpha:
                    return VK_BLEND_FACTOR_DST_ALPHA;
                case RHIBlendFactor::InvDstAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                case RHIBlendFactor::ConstantColor:
                    return VK_BLEND_FACTOR_CONSTANT_COLOR;
                case RHIBlendFactor::InvConstantColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
                case RHIBlendFactor::ConstantAlpha:
                    return VK_BLEND_FACTOR_CONSTANT_ALPHA;
                case RHIBlendFactor::InvConstantAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
                case RHIBlendFactor::SrcAlphaSat:
                    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
                case RHIBlendFactor::Src1Color:
                    return VK_BLEND_FACTOR_SRC1_COLOR;
                case RHIBlendFactor::InvSrc1Color:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
                case RHIBlendFactor::Src1Alpha:
                    return VK_BLEND_FACTOR_SRC1_ALPHA;
                case RHIBlendFactor::InvSrc1Alpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            }
            unreachable();
        }

        VkBlendOp to_vk_blend_op(RHIBlendOp blend_op)
        {
            switch (blend_op) {
                case RHIBlendOp::Add:
                    return VK_BLEND_OP_ADD;
                case RHIBlendOp::Subtract:
                    return VK_BLEND_OP_SUBTRACT;
                case RHIBlendOp::ReverseSubtract:
                    return VK_BLEND_OP_REVERSE_SUBTRACT;
                case RHIBlendOp::Min:
                    return VK_BLEND_OP_MIN;
                case RHIBlendOp::Max:
                    return VK_BLEND_OP_MAX;
            }
            unreachable();
        }

        VkPipelineStageFlags to_vk_stage_flags(RHIImageLayout layout)
        {
            switch (layout) {
                case RHIImageLayout::Undefined:
                    return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                case RHIImageLayout::RenderTarget:
                    return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                case RHIImageLayout::PresentSrc:
                    return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }
            unreachable();
        }

        VkAccessFlags to_vk_access_flags(RHIImageLayout layout)
        {
            switch (layout) {
                case RHIImageLayout::Undefined:
                    return {};
                case RHIImageLayout::RenderTarget:
                    return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                case RHIImageLayout::PresentSrc:
                    return {};
            }
            unreachable();
        }

        VkImageLayout to_vk_image_layout(RHIImageLayout layout)
        {
            switch (layout) {
                case RHIImageLayout::Undefined:
                    return VK_IMAGE_LAYOUT_UNDEFINED;
                case RHIImageLayout::RenderTarget:
                    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                case RHIImageLayout::PresentSrc:
                    return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
            unreachable();
        }

        VkBool32 vulkan_debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
            void* /*pUserData*/)
        {
            if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
                ORION_CORE_LOG_ERROR("Vulkan validation error ({}): {}", callback_data->pMessageIdName, callback_data->pMessage);
            } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                ORION_CORE_LOG_ERROR("Vulkan validation warning ({}): {}", callback_data->pMessageIdName, callback_data->pMessage);
            } else {
                ORION_CORE_LOG_TRACE("Vulkan validation message ({}): {}", callback_data->pMessageIdName, callback_data->pMessage);
            }
            return VK_FALSE;
        }

        class RHIVulkanCommandAllocator : public RHICommandAllocator
        {
        public:
            RHIVulkanCommandAllocator(VkDevice device, VkCommandPool command_pool)
                : device_(device)
                , command_pool_(command_pool)
            {
                ORION_ASSERT(device != VK_NULL_HANDLE, "VkDevice must not be VK_NULL_HANDLE");
                ORION_ASSERT(command_pool != VK_NULL_HANDLE, "VkCommandPool must not be VK_NULL_HANDLE");
            }

            ~RHIVulkanCommandAllocator() override
            {
                vkDestroyCommandPool(device_, command_pool_, nullptr);
                ORION_CORE_LOG_INFO("Destroyed VkCommandPool {}", (void*)command_pool_);
            }

            [[nodiscard]] VkCommandPool command_pool() const noexcept { return command_pool_; }

        private:
            bool reset_api() override
            {
                if (VkResult err = vkResetCommandPool(device_, command_pool_, {})) {
                    ORION_CORE_LOG_ERROR("Failed to reset VkCommandPool {}: {}", (void*)command_pool_, string_VkResult(err));
                    return false;
                } else {
                    return true;
                }
            }

            VkDevice device_;
            VkCommandPool command_pool_;
        };

        class RHIVulkanCommandList : public RHICommandList
        {
        public:
            RHIVulkanCommandList(
                VkDevice device,
                VkCommandPool command_pool,
                VkCommandBuffer command_buffer,
                VulkanResourceTable* resources)
                : device_(device)
                , command_pool_(command_pool)
                , command_buffer_(command_buffer)
                , resources_(resources)
            {
                ORION_ASSERT(device != VK_NULL_HANDLE, "VkDevice must not be VK_NULL_HANDLE");
                ORION_ASSERT(command_pool != VK_NULL_HANDLE, "VkCommandPool must not be VK_NULL_HANDLE");
                ORION_ASSERT(command_buffer != VK_NULL_HANDLE, "VkCommandBuffer must not be VK_NULL_HANDLE");
                ORION_ASSERT(resources != VK_NULL_HANDLE, "VulkanResourceTable must not be nullptr");
            }

            ~RHIVulkanCommandList() override
            {
                vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer_);
                ORION_CORE_LOG_INFO("Freed VkCommandBuffer {}", (void*)command_buffer_);
            }

            [[nodiscard]] VkCommandBuffer vk_command_buffer() const noexcept { return command_buffer_; }

        private:
            bool reset_api() override
            {
                if (VkResult err = vkResetCommandBuffer(command_buffer_, {})) {
                    ORION_CORE_LOG_ERROR("Failed to reset VkCommandBuffer {}: {}", (void*)command_buffer_, string_VkResult(err));
                    return false;
                } else {
                    return true;
                }
            }

            void begin_api() override
            {
                const auto begin_info = VkCommandBufferBeginInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .pNext = nullptr,
                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // Profile if this makes a difference
                    .pInheritanceInfo = nullptr,
                };
                if (VkResult err = vkBeginCommandBuffer(command_buffer_, &begin_info)) {
                    throw std::runtime_error(fmt::format("vkBeginCommandBuffer failed: {}", string_VkResult(err)));
                }
            }

            void end_api() override
            {
                if (VkResult err = vkEndCommandBuffer(command_buffer_)) {
                    throw std::runtime_error(fmt::format("vkEndCommandBuffer failed: {}", string_VkResult(err)));
                }
            }

            void pipeline_barrier_api(const RHICmdPipelineBarrier& cmd) override
            {
                VkPipelineStageFlags src_stage_mask = {};
                VkPipelineStageFlags dst_stage_mask = {};

                std::vector<VkImageMemoryBarrier> image_barriers(cmd.transition_barriers.size());
                std::ranges::transform(cmd.transition_barriers, image_barriers.begin(), [&](const RHITransitionBarrier& barrier) {
                    // Find image resource & update it's state
                    auto* vulkan_image = resources_->images.get(barrier.image.value);
                    ORION_ASSERT(vulkan_image != nullptr, "RHIImage must be valid");
                    vulkan_image->current_layout = to_vk_image_layout(barrier.new_layout);

                    // Update pipeline stage flags
                    src_stage_mask |= to_vk_stage_flags(barrier.old_layout);
                    dst_stage_mask |= to_vk_stage_flags(barrier.new_layout);

                    return VkImageMemoryBarrier{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .pNext = nullptr,
                        .srcAccessMask = to_vk_access_flags(barrier.old_layout),
                        .dstAccessMask = to_vk_access_flags(barrier.new_layout),
                        .oldLayout = to_vk_image_layout(barrier.old_layout),
                        .newLayout = vulkan_image->current_layout,
                        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .image = vulkan_image->image,
                        .subresourceRange = {
                            .aspectMask = vulkan_image->aspect_flags(),
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        },
                    };
                });

                vkCmdPipelineBarrier(
                    command_buffer_,
                    src_stage_mask,                                                          // VkDependencyFlags
                    dst_stage_mask,                                                          // VkDependencyFlags
                    {},                                                                      // VkDependencyFlags
                    0, nullptr,                                                              // VkMemoryBarrier
                    0, nullptr,                                                              // VkBufferMemoryBarrier
                    static_cast<std::uint32_t>(image_barriers.size()), image_barriers.data() // VkImageMemoryBarrier
                );
            }

            void begin_rendering_api(const RHICmdBeginRendering& cmd) override
            {
                std::vector<VkRenderingAttachmentInfo> color_attachments(cmd.rtvs.size());
                std::ranges::transform(cmd.rtvs, color_attachments.begin(), [&](RHIImageView image_view) {
                    const auto* vulkan_image_view = resources_->image_views.get(image_view.value);
                    ORION_ASSERT(vulkan_image_view != nullptr, "RHIImageView must be a valid handle");
                    return VkRenderingAttachmentInfo{
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .pNext = nullptr,
                        .imageView = vulkan_image_view->image_view,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = VK_NULL_HANDLE,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .clearValue = {.color = {{cmd.rtv_clear[0], cmd.rtv_clear[1], cmd.rtv_clear[2], cmd.rtv_clear[3]}}},
                    };
                });
                VkRenderingAttachmentInfo depth_attachment;
                if (cmd.dsv.is_valid()) {
                    const auto* vulkan_image_view = resources_->image_views.get(cmd.dsv.value);
                    ORION_ASSERT(vulkan_image_view != nullptr, "RHIImageView must be a valid handle");
                    depth_attachment = {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .pNext = nullptr,
                        .imageView = vulkan_image_view->image_view,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = VK_NULL_HANDLE,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .clearValue = {.depthStencil = {.depth = cmd.depth_clear}},
                    };
                }

                const auto rendering_info = VkRenderingInfo{
                    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .renderArea = {
                        .offset = {},
                        .extent = {cmd.render_width, cmd.render_height},
                    },
                    .layerCount = 1,
                    .viewMask = 0,
                    .colorAttachmentCount = static_cast<std::uint32_t>(cmd.rtvs.size()),
                    .pColorAttachments = color_attachments.data(),
                    .pDepthAttachment = cmd.dsv.is_valid() ? &depth_attachment : nullptr,
                    .pStencilAttachment = nullptr,
                };
                vkCmdBeginRendering(command_buffer_, &rendering_info);
            }

            void end_rendering_api() override
            {
                vkCmdEndRendering(command_buffer_);
            }

            void set_graphics_pipeline_state_api(RHIPipeline pipeline) override
            {
                const auto* vulkan_pipeline = resources_->pipelines.get(pipeline.value);
                ORION_ASSERT(vulkan_pipeline != nullptr, "RHIPipeline must be a valid handle");
                vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_pipeline->pipeline);
            }

            void draw_instanced_api(const RHICmdDrawInstanced& cmd) override
            {
                vkCmdDraw(command_buffer_, cmd.vertex_count, cmd.instance_count, cmd.first_vertex, cmd.first_instance);
            }

            void set_viewports_api(const RHICmdSetViewports& cmd) override
            {
                std::vector<VkViewport> viewports(cmd.viewports.size());
                std::ranges::transform(cmd.viewports, viewports.begin(), [](const RHIViewport& viewport) {
                    return VkViewport{
                        .x = viewport.x,
                        .y = viewport.y,
                        .width = viewport.width,
                        .height = viewport.height,
                        .minDepth = viewport.min_depth,
                        .maxDepth = viewport.max_depth,
                    };
                });
                vkCmdSetViewport(command_buffer_, cmd.first_viewport, static_cast<std::uint32_t>(cmd.viewports.size()), viewports.data());
            }

            void set_scissors_api(const RHICmdSetScissors& cmd) override
            {
                std::vector<VkRect2D> scissors(cmd.scissors.size());
                std::ranges::transform(cmd.scissors, scissors.begin(), [](const RHIRect& rect) {
                    return VkRect2D{
                        .offset = {
                            .x = rect.left,
                            .y = rect.top,
                        },
                        .extent = {
                            .width = static_cast<std::uint32_t>(rect.right - rect.left),
                            .height = static_cast<std::uint32_t>(rect.bottom - rect.top),
                        },
                    };
                });
                vkCmdSetScissor(command_buffer_, 0, static_cast<std::uint32_t>(cmd.scissors.size()), scissors.data());
            }

            VkDevice device_;
            VkCommandPool command_pool_;
            VkCommandBuffer command_buffer_;
            VulkanResourceTable* resources_;
        };

        class RHIVulkanCommandQueue : public RHICommandQueue
        {
        public:
            RHIVulkanCommandQueue(VkQueue queue, std::uint32_t queue_family, VulkanResourceTable* resources)
                : queue_(queue)
                , queue_family_(queue_family)
                , resources_(resources)
            {
                ORION_ASSERT(queue != VK_NULL_HANDLE, "VkQueue must not be VK_NULL_HANDLE");
                ORION_ASSERT(queue_family != UINT32_MAX, "Queue family must not be UINT32_MAX");
                ORION_ASSERT(resources != nullptr, "VulkanResourceTable must not be nullptr");
            }

            [[nodiscard]] VkQueue vk_queue() const noexcept { return queue_; }

        private:
            void wait_api(RHISemaphore semaphore) override
            {
                const auto* vk_semaphore = resources_->semaphores.get(semaphore.value);
                ORION_ASSERT(vk_semaphore != nullptr, "RHISemaphore must be a valid handle");
                wait_semaphores_.push_back(*vk_semaphore);
                wait_stages_.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT); // Use queue type to refine this
            }

            void signal_api(RHISemaphore semaphore) override
            {
                const auto* vk_semaphore = resources_->semaphores.get(semaphore.value);
                ORION_ASSERT(vk_semaphore != nullptr, "RHISemaphore must be a valid handle");
                signal_semaphores_.push_back(*vk_semaphore);
            }

            void submit_api(std::span<const RHICommandList* const> command_lists, RHIFence fence) override
            {
                // Get VkCommandBuffer's
                std::vector<VkCommandBuffer> command_buffers(command_lists.size());
                std::ranges::transform(command_lists, command_buffers.begin(), [](const RHICommandList* command_list) {
                    const auto* vulkan_command_list = dynamic_cast<const RHIVulkanCommandList*>(command_list);
                    ORION_ASSERT(vulkan_command_list != nullptr, "All RHICommandList's must be valid Vulkan command lists");
                    return vulkan_command_list->vk_command_buffer();
                });

                // If RHIFence is a valid handle, get VkFence
                VkFence vk_fence = VK_NULL_HANDLE;
                if (fence.is_valid()) {
                    const auto* vulkan_fence = resources_->fences.get(fence.value);
                    ORION_ASSERT(vulkan_fence != nullptr, "If RHIFence was not RHIFence::invalid() it must be a valid handle");
                    vk_fence = *vulkan_fence;
                }

                // Submit command buffers
                const auto submit_info = VkSubmitInfo{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .pNext = nullptr,
                    .waitSemaphoreCount = static_cast<std::uint32_t>(wait_semaphores_.size()),
                    .pWaitDstStageMask = wait_stages_.data(),
                    .commandBufferCount = static_cast<std::uint32_t>(command_buffers.size()),
                    .pCommandBuffers = command_buffers.data(),
                    .signalSemaphoreCount = static_cast<std::uint32_t>(signal_semaphores_.size()),
                    .pSignalSemaphores = signal_semaphores_.data(),
                };
                if (VkResult err = vkQueueSubmit(queue_, 1, &submit_info, vk_fence)) {
                    throw std::runtime_error(fmt::format("vkQueueSubmit failed: {}", string_VkResult(err)));
                }

                // Reset list of wait & signal semaphores
                wait_semaphores_.clear();
                wait_stages_.clear();
                signal_semaphores_.clear();
            }

            VkQueue queue_;
            std::uint32_t queue_family_;
            VulkanResourceTable* resources_;

            std::vector<VkSemaphore> wait_semaphores_;
            std::vector<VkPipelineStageFlags> wait_stages_;
            std::vector<VkSemaphore> signal_semaphores_;
        };

        class RHIVulkanDevice : public RHIDevice
        {
        public:
            RHIVulkanDevice(
                VkInstance instance,
                VkPhysicalDevice physical_device,
                VkDevice device,
                VulkanQueue graphics_queue)
                : instance_(instance)
                , physical_device_(physical_device)
                , device_(device)
                , graphics_queue_(graphics_queue)
            {
                ORION_ASSERT(instance != VK_NULL_HANDLE, "VkInstance must not be VK_NULL_HANDLE");
                ORION_ASSERT(physical_device != VK_NULL_HANDLE, "VkPhysicalDevice must not be VK_NULL_HANDLE");
                ORION_ASSERT(device != VK_NULL_HANDLE, "VkDevice must not be VK_NULL_HANDLE");
                ORION_ASSERT(graphics_queue.queue != VK_NULL_HANDLE, "VkQueue (graphics) must not be VK_NULL_HANDLE");
            }

            ~RHIVulkanDevice() override
            {
                vkDeviceWaitIdle(device_);
                vkDestroyDevice(device_, nullptr);
                ORION_CORE_LOG_INFO("Destroyed VkDevice {}", (void*)device_);
            }

        private:
            std::unique_ptr<RHICommandQueue> create_command_queue_api(const RHICommandQueueDesc& desc) override
            {
                switch (desc.type) {
                    case RHICommandQueueType::Graphics:
                        return std::make_unique<RHIVulkanCommandQueue>(graphics_queue_.queue, graphics_queue_.family, &resources_);
                }
                unreachable();
            }

            std::unique_ptr<RHICommandAllocator> create_command_allocator_api(const RHICommandAllocatorDesc& desc) override
            {
                const auto queue_family_index = get_queue_family_index(desc.type);
                const auto command_pool_info = VkCommandPoolCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                    .queueFamilyIndex = queue_family_index,
                };
                VkCommandPool command_pool = VK_NULL_HANDLE;
                if (VkResult err = vkCreateCommandPool(device_, &command_pool_info, nullptr, &command_pool)) {
                    ORION_CORE_LOG_ERROR("Failed to create Vulkan command pool: {}", string_VkResult(err));
                    return nullptr;
                }
                ORION_CORE_LOG_INFO("Created VkCommandPool {} with queueFamilyIndex = {}", (void*)command_pool, queue_family_index);
                return std::make_unique<RHIVulkanCommandAllocator>(device_, command_pool);
            }

            std::unique_ptr<RHICommandList> create_command_list_api(const RHICommandListDesc& desc) override
            {
                const auto* vulkan_command_allocator = dynamic_cast<const RHIVulkanCommandAllocator*>(desc.command_allocator);
                if (!vulkan_command_allocator) {
                    ORION_CORE_LOG_ERROR("Invalid Vulkan RHICommandAllocator* {}", (void*)desc.command_allocator);
                    return nullptr;
                }
                VkCommandPool command_pool = vulkan_command_allocator->command_pool();

                const auto command_buffer_info = VkCommandBufferAllocateInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .commandPool = command_pool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = 1,
                };
                VkCommandBuffer command_buffer = VK_NULL_HANDLE;
                if (VkResult err = vkAllocateCommandBuffers(device_, &command_buffer_info, &command_buffer)) {
                    ORION_CORE_LOG_ERROR("Failed to allocate Vulkan command buffer: {}", string_VkResult(err));
                    return nullptr;
                }
                ORION_CORE_LOG_INFO("Allocated VkCommandBuffer {} from VkCommandPool {}", (void*)command_buffer, (void*)command_pool);
                return std::make_unique<RHIVulkanCommandList>(device_, command_pool, command_buffer, &resources_);
            }

            RHISwapchain create_swapchain_api(const RHISwapchainDesc& desc) override
            {
                // Get Vulkan queue
                auto* vulkan_queue = dynamic_cast<RHIVulkanCommandQueue*>(desc.queue);
                ORION_ASSERT(vulkan_queue != nullptr, "RHIcommandQueue must be a valid Vulkan command queue");

                // Create surface
                GLFWwindow* window = desc.window->window;
                VkSurfaceKHR surface = VK_NULL_HANDLE;
                if (VkResult err = glfwCreateWindowSurface(instance_, window, nullptr, &surface)) {
                    ORION_CORE_LOG_ERROR("Failed to create VkSurface with GLFWwindow* {}: {}", (void*)window, string_VkResult(err));
                    return RHISwapchain::invalid();
                }
                ORION_CORE_LOG_INFO("Created VkSurfaceKHR {} with GLFWwindow* {}", (void*)surface, (void*)window);

                // Get surface capabilties
                VkSurfaceCapabilitiesKHR surface_capabilities;
                if (VkResult err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface, &surface_capabilities)) {
                    ORION_CORE_LOG_ERROR("Failed to get VkSurfaceKHR ({}) surface capabilities: {}", (void*)surface, string_VkResult(err));
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return RHISwapchain::invalid();
                }

                // Check if requested dimensions match currentExtent
                if (desc.width != surface_capabilities.currentExtent.width || desc.height != surface_capabilities.currentExtent.height) {
                    ORION_CORE_LOG_ERROR("Requested dimensions ({}, {}) do not match currentExtent ({}, {}) of VkSurfaceKHR ({})",
                                         desc.width, desc.height,
                                         surface_capabilities.currentExtent.width, surface_capabilities.currentExtent.height,
                                         (void*)surface);
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return RHISwapchain::invalid();
                }

                // Check if requested image count is supported
                auto image_count = desc.image_count;
                if (image_count < surface_capabilities.minImageCount || image_count > surface_capabilities.maxImageCount) {
                    ORION_CORE_LOG_ERROR("Requested image count ({}) is not in range of supported image counts [{}, {}]",
                                         image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return RHISwapchain::invalid();
                }
                ORION_CORE_LOG_DEBUG("Using image_count = {} for VkSwapchainKHR", image_count);

                // Get list of supported surface formats
                std::uint32_t surface_format_count = 0;
                if (VkResult err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &surface_format_count, nullptr)) {
                    ORION_CORE_LOG_ERROR("Failed to get VkPhysicalDevice {} surface formats for VkSurfaceKHR {}: {}",
                                         (void*)physical_device_, (void*)surface, string_VkResult(err));
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return RHISwapchain::invalid();
                }
                std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
                if (VkResult err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &surface_format_count, surface_formats.data())) {
                    ORION_CORE_LOG_ERROR("Failed to get VkPhysicalDevice {} surface formats for VkSurfaceKHR {}: {}",
                                         (void*)physical_device_, (void*)surface, string_VkResult(err));
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return RHISwapchain::invalid();
                }

                // Check if requested surface format is supported
                const auto format = to_vk_format(desc.format);
                constexpr auto colorspace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
                const auto format_cmp = [format](const VkSurfaceFormatKHR& surface_format) {
                    return surface_format.format == format && surface_format.colorSpace == colorspace;
                };
                if (auto iter = std::ranges::find_if(surface_formats, format_cmp); iter == surface_formats.end()) {
                    ORION_CORE_LOG_ERROR("Requested surface format ({}, {}) is not supported for VkSurfaceKHR {}",
                                         string_VkFormat(format), string_VkColorSpaceKHR(colorspace), (void*)surface);
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return RHISwapchain::invalid();
                }
                ORION_CORE_LOG_DEBUG("Using format = {}, {} for VkSwapchainKHR", string_VkFormat(format), string_VkColorSpaceKHR(colorspace));

                const auto swapchain_info = VkSwapchainCreateInfoKHR{
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .pNext = nullptr,
                    .flags = {},
                    .surface = surface,
                    .minImageCount = image_count,
                    .imageFormat = format,
                    .imageColorSpace = colorspace,
                    .imageExtent = surface_capabilities.currentExtent,
                    .imageArrayLayers = 1,
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // TODO: Make this customizable
                    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 0,
                    .pQueueFamilyIndices = nullptr,
                    .preTransform = surface_capabilities.currentTransform,
                    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                    .presentMode = VK_PRESENT_MODE_FIFO_KHR, // TODO: Make this customizable
                    .clipped = VK_TRUE,
                };
                VkSwapchainKHR swapchain = VK_NULL_HANDLE;
                if (VkResult err = vkCreateSwapchainKHR(device_, &swapchain_info, nullptr, &swapchain)) {
                    ORION_CORE_LOG_ERROR("Failed to create VkSwapchainKHR with VkSurface {}: {}", (void*)surface, string_VkResult(err));
                    return RHISwapchain::invalid();
                }
                ORION_CORE_LOG_INFO("Created VkSwapchainKHR {}", (void*)swapchain);

                // Acquire swapchain images
                std::vector<VkImage> images(image_count);
                if (VkResult err = vkGetSwapchainImagesKHR(device_, swapchain, &image_count, images.data())) {
                    ORION_CORE_LOG_ERROR("Failed to acquire VkSwapchainKHR {} images: {}", (void*)swapchain, string_VkResult(err));
                    vkDestroySwapchainKHR(device_, swapchain, nullptr);
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                }

                // Add swapchain images to handle table
                std::vector<RHIImage> image_handles(image_count);
                std::ranges::transform(images, image_handles.begin(), [&](VkImage image) {
                    const auto handle = resources_.images.insert(VulkanImage{image});
                    return RHIImage{handle.as_uint64_t()};
                });

                const auto handle = resources_.swapchains.insert(VulkanSwapchain{surface, swapchain, vulkan_queue, std::move(image_handles)});
                return RHISwapchain{handle.as_uint64_t()};
            }

            RHIPipeline create_graphics_pipeline_api(const RHIGraphicsPipelineDesc& desc) override
            {
                // Pipeline shaders
                VkShaderModule vertex_shader = VK_NULL_HANDLE;
                VkShaderModule fragment_shader = VK_NULL_HANDLE;
                const auto finally = Finally{[&] {
                    if (vertex_shader != VK_NULL_HANDLE) {
                        vkDestroyShaderModule(device_, vertex_shader, nullptr);
                    }
                    if (fragment_shader != VK_NULL_HANDLE) {
                        vkDestroyShaderModule(device_, fragment_shader, nullptr);
                    }
                }};

                if (VkResult err = create_shader_module(desc.VS, &vertex_shader)) {
                    ORION_CORE_LOG_ERROR("Failed to create Vulkan graphics pipeline vertex shader: {}", string_VkResult(err));
                }
                if (VkResult err = create_shader_module(desc.FS, &fragment_shader)) {
                    ORION_CORE_LOG_ERROR("Failed to create Vulkan graphics pipeline fragment shader: {}", string_VkResult(err));
                }
                const auto shader_stage_infos = std::array{
                    VkPipelineShaderStageCreateInfo{
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = {},
                        .stage = VK_SHADER_STAGE_VERTEX_BIT,
                        .module = vertex_shader,
                        .pName = "main",
                        .pSpecializationInfo = nullptr,
                    },
                    VkPipelineShaderStageCreateInfo{
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = {},
                        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .module = fragment_shader,
                        .pName = "main",
                        .pSpecializationInfo = nullptr,
                    },
                };

                // Pipeline vertex input
                std::vector<VkVertexInputBindingDescription> vertex_bindings;
                std::vector<VkVertexInputAttributeDescription> vertex_attributes;
                for (std::uint32_t binding = 0; binding < desc.vertex_bindings.size(); ++binding) {
                    const auto attributes = desc.vertex_bindings[binding].attributes;
                    std::uint32_t stride = 0;
                    for (std::uint32_t location = 0; location < attributes.size(); ++location) {
                        VkFormat format = to_vk_format(attributes[location].format);
                        vertex_attributes.push_back(VkVertexInputAttributeDescription{
                            .location = location,
                            .binding = binding,
                            .format = format,
                            .offset = stride,
                        });
                        stride += vk_format_size(format);
                    }
                    vertex_bindings.push_back(VkVertexInputBindingDescription{
                        .binding = binding,
                        .stride = stride,
                        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // TODO: Make this customizable
                    });
                }
                const auto vertex_input_info = VkPipelineVertexInputStateCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .vertexBindingDescriptionCount = static_cast<std::uint32_t>(vertex_bindings.size()),
                    .pVertexBindingDescriptions = vertex_bindings.data(),
                    .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertex_attributes.size()),
                    .pVertexAttributeDescriptions = vertex_attributes.data(),
                };

                // Pipeline input assembly
                const auto input_assembly_info = VkPipelineInputAssemblyStateCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .topology = to_vk_primitive_topology(desc.input_assembly.topology),
                    .primitiveRestartEnable = VK_FALSE,
                };

                // Pipeline tesselation
                const auto tesselation_info = VkPipelineTessellationStateCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .patchControlPoints = 0,
                };

                // Pipeline viewport
                // TODO: Make viewport & scissor count customizable
                const auto viewport_info = VkPipelineViewportStateCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .viewportCount = 1,
                    .pViewports = nullptr,
                    .scissorCount = 1,
                    .pScissors = nullptr,
                };

                // Pipeline rasterization
                const auto rasterization_info = VkPipelineRasterizationStateCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .depthClampEnable = VK_FALSE,
                    .rasterizerDiscardEnable = VK_FALSE,
                    .polygonMode = to_vk_polygon_mode(desc.rasterizer.fill_mode),
                    .cullMode = to_vk_cull_mode(desc.rasterizer.cull_mode),
                    .frontFace = desc.rasterizer.front_face == RHIFrontFace::CounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE,
                    .depthBiasEnable = VK_FALSE,
                    .depthBiasConstantFactor = 0.0f,
                    .depthBiasClamp = 0.0f,
                    .depthBiasSlopeFactor = 0.0f,
                    .lineWidth = 1.0f,
                };

                // Pipeline multisampling
                const auto multisample_info = VkPipelineMultisampleStateCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                    .sampleShadingEnable = VK_FALSE,
                    .minSampleShading = 0.0f,
                    .pSampleMask = nullptr,
                    .alphaToCoverageEnable = VK_FALSE,
                    .alphaToOneEnable = VK_FALSE,
                };

                // Pipline depth-stencil
                const auto depth_stencil_info = VkPipelineDepthStencilStateCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .depthTestEnable = desc.depth_stencil.depth_enable,
                    .depthWriteEnable = desc.depth_stencil.depth_write_enable,
                    .depthCompareOp = to_vk_compare_op(desc.depth_stencil.compare_op),
                    .depthBoundsTestEnable = VK_FALSE,
                    .stencilTestEnable = VK_FALSE,
                };

                // Pipeline color blend
                std::vector<VkPipelineColorBlendAttachmentState> blend_attachments(desc.blend.render_targets.size());
                std::ranges::transform(desc.blend.render_targets, blend_attachments.begin(), [](const RHIRenderTargetBlendDesc& desc) {
                    return VkPipelineColorBlendAttachmentState{
                        .blendEnable = desc.blend_enable,
                        .srcColorBlendFactor = to_vk_blend_factor(desc.src_blend),
                        .dstColorBlendFactor = to_vk_blend_factor(desc.dst_blend),
                        .colorBlendOp = to_vk_blend_op(desc.blend_op),
                        .srcAlphaBlendFactor = to_vk_blend_factor(desc.src_alpha_blend),
                        .dstAlphaBlendFactor = to_vk_blend_factor(desc.dst_alpha_blend),
                        .alphaBlendOp = to_vk_blend_op(desc.alpha_blend_op),
                        .colorWriteMask = static_cast<VkColorComponentFlags>(desc.color_write_mask),
                    };
                });
                const auto blend_info = VkPipelineColorBlendStateCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT,
                    .pNext = nullptr,
                    .flags = {},
                    .logicOpEnable = VK_FALSE,
                    .attachmentCount = static_cast<std::uint32_t>(blend_attachments.size()),
                    .pAttachments = blend_attachments.data(),
                    .blendConstants = {
                        desc.blend.blend_constants[0],
                        desc.blend.blend_constants[1],
                        desc.blend.blend_constants[2],
                        desc.blend.blend_constants[3],
                    },
                };

                // Pipeline dynamic state
                const auto dynamic_states = std::array{
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR,
                };
                const auto dynamic_state_info = VkPipelineDynamicStateCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
                    .pDynamicStates = dynamic_states.data(),
                };

                // Pipeline dynamic rendering
                std::vector<VkFormat> color_attachment_formats(desc.rtv_formats.size());
                std::ranges::transform(desc.rtv_formats, color_attachment_formats.begin(), to_vk_format);
                const auto depth_stencil_format = to_vk_format(desc.dsv_format);
                const auto rendering_info = VkPipelineRenderingCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                    .pNext = nullptr,
                    .viewMask = 0,
                    .colorAttachmentCount = static_cast<std::uint32_t>(color_attachment_formats.size()),
                    .pColorAttachmentFormats = color_attachment_formats.data(),
                    .depthAttachmentFormat = depth_stencil_format,
                    .stencilAttachmentFormat = depth_stencil_format,
                };

                // Pipeline layout
                // TODO: Implement single bindless layout
                const auto layout_info = VkPipelineLayoutCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    .pNext = nullptr,
                };
                VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
                if (VkResult err = vkCreatePipelineLayout(device_, &layout_info, nullptr, &pipeline_layout)) {
                    ORION_CORE_LOG_ERROR("Failed to create Vulkan pipeline layout: {}", string_VkResult(err));
                    return RHIPipeline::invalid();
                }
                ORION_CORE_LOG_INFO("Created VkPipelineLayout (placeholder) {}", (void*)pipeline_layout);

                // Pipeline
                const auto pipeline_info = VkGraphicsPipelineCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                    .pNext = &rendering_info,
                    .flags = {},
                    .stageCount = static_cast<std::uint32_t>(shader_stage_infos.size()),
                    .pStages = shader_stage_infos.data(),
                    .pVertexInputState = &vertex_input_info,
                    .pInputAssemblyState = &input_assembly_info,
                    .pTessellationState = &tesselation_info,
                    .pViewportState = &viewport_info,
                    .pRasterizationState = &rasterization_info,
                    .pMultisampleState = &multisample_info,
                    .pDepthStencilState = &depth_stencil_info,
                    .pColorBlendState = &blend_info,
                    .pDynamicState = &dynamic_state_info,
                    .layout = pipeline_layout,
                };
                VkPipeline pipeline = VK_NULL_HANDLE;
                if (VkResult err = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline)) {
                    ORION_CORE_LOG_ERROR("Failed to create Vulkan graphics pipeline: {}", string_VkResult(err));
                    vkDestroyPipelineLayout(device_, pipeline_layout, nullptr);
                    return RHIPipeline::invalid();
                }
                ORION_CORE_LOG_INFO("Created VkPipeline (graphics) {}", (void*)pipeline);

                const auto handle = resources_.pipelines.insert(VulkanPipeline{pipeline, pipeline_layout});
                return RHIPipeline{handle.as_uint64_t()};
            }

            RHISemaphore create_semaphore_api(const RHISemaphoreDesc& /*desc*/) override
            {
                const auto semaphore_info = VkSemaphoreCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                };
                VkSemaphore semaphore = VK_NULL_HANDLE;
                if (VkResult err = vkCreateSemaphore(device_, &semaphore_info, nullptr, &semaphore)) {
                    ORION_CORE_LOG_ERROR("Failed to create Vulkan semaphore: {}", string_VkResult(err));
                    return RHISemaphore::invalid();
                }
                ORION_CORE_LOG_INFO("Created VkSemaphore {}", (void*)semaphore);

                const auto handle = resources_.semaphores.insert(semaphore);
                return RHISemaphore{handle.as_uint64_t()};
            }

            RHIFence create_fence_api(const RHIFenceDesc& desc) override
            {
                const auto fence_info = VkFenceCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = desc.create_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags{},
                };
                VkFence fence = VK_NULL_HANDLE;
                if (VkResult err = vkCreateFence(device_, &fence_info, nullptr, &fence)) {
                    ORION_CORE_LOG_ERROR("Failed to create Vulkan fence: {}", string_VkResult(err));
                    return RHIFence::invalid();
                }
                ORION_CORE_LOG_INFO("Created VkFence {}", (void*)fence);

                const auto handle = resources_.fences.insert(fence);
                return RHIFence{handle.as_uint64_t()};
            }

            RHIImageView create_render_target_view_api(const RHIRenderTargetViewDesc& desc) override
            {
                const auto* image = resources_.images.get(desc.image.value);
                if (!image) {
                    ORION_CORE_LOG_ERROR("Invalid RHIImage {} when creating render target view", desc.image.value);
                    return RHIImageView::invalid();
                }

                const auto image_view_info = VkImageViewCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .image = image->image,
                    .viewType = VK_IMAGE_VIEW_TYPE_2D, // TODO: Make this customizable
                    .format = to_vk_format(desc.format),
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
                if (VkResult err = vkCreateImageView(device_, &image_view_info, nullptr, &image_view)) {
                    ORION_CORE_LOG_ERROR("Failed to create Vulkan render target image view: {}", string_VkResult(err));
                    return RHIImageView::invalid();
                }
                ORION_CORE_LOG_INFO("Created VkImageView (rtv) {}", (void*)image_view);

                const auto handle = resources_.image_views.insert(VulkanImageView{image_view});
                return RHIImageView{handle.as_uint64_t()};
            }

            void destroy_api(RHISwapchain handle) override
            {
                if (const auto* swapchain = resources_.swapchains.get(handle.value)) {
                    // Release resource handles
                    for (RHIImage image : swapchain->images) {
                        resources_.images.remove(image.value);
                    }
                    resources_.swapchains.remove(handle.value);

                    // Destroy resources
                    vkDestroySwapchainKHR(device_, swapchain->swapchain, nullptr);
                    vkDestroySurfaceKHR(instance_, swapchain->surface, nullptr);
                    ORION_CORE_LOG_INFO("Destroyed VkSwapchainKHR {}", (void*)swapchain->swapchain);
                    ORION_CORE_LOG_INFO("Destroyed VkSurfaceKHR {}", (void*)swapchain->surface);
                } else {
                    ORION_CORE_LOG_WARN("Attempting to destroy RHISwapchain ({}) which not a valid Vulkan handle", handle.value);
                }
            }

            void destroy_api(RHIPipeline handle) override
            {
                if (const auto* pipeline = resources_.pipelines.get(handle.value)) {
                    // Release resource handles
                    resources_.pipelines.remove(handle.value);

                    // Destroy resources
                    vkDestroyPipeline(device_, pipeline->pipeline, nullptr);
                    vkDestroyPipelineLayout(device_, pipeline->layout, nullptr);
                    ORION_CORE_LOG_INFO("Destroyed VkPipeline {}", (void*)pipeline->pipeline);
                    ORION_CORE_LOG_INFO("Destroyed VkPipelineLayout {}", (void*)pipeline->layout);
                } else {
                    ORION_CORE_LOG_WARN("Attempting to destroy RHIPipeline ({}) which not a valid Vulkan handle", handle.value);
                }
            }

            void destroy_api(RHISemaphore handle) override
            {
                if (const auto* semaphore = resources_.semaphores.get(handle.value)) {
                    // Release resource handles
                    resources_.semaphores.remove(handle.value);

                    // Destroy resources
                    vkDestroySemaphore(device_, *semaphore, nullptr);
                    ORION_CORE_LOG_INFO("Destroyed VkSemaphore {}", (void*)*semaphore);
                } else {
                    ORION_CORE_LOG_WARN("Attempting to destroy RHISemaphore ({}) which not a valid Vulkan handle", handle.value);
                }
            }

            void destroy_api(RHIFence handle) override
            {
                if (const auto* fence = resources_.fences.get(handle.value)) {
                    // Release resource handles
                    resources_.fences.remove(handle.value);

                    // Destroy resources
                    vkDestroyFence(device_, *fence, nullptr);
                    ORION_CORE_LOG_INFO("Destroyed VkFence {}", (void*)*fence);
                } else {
                    ORION_CORE_LOG_WARN("Attempting to destroy RHIFence ({}) which not a valid Vulkan handle", handle.value);
                }
            }

            void destroy_api(RHIImageView handle) override
            {
                if (const auto* image_view = resources_.image_views.get(handle.value)) {
                    // Release resource handles
                    resources_.image_views.remove(handle.value);

                    // Destroy resources
                    vkDestroyImageView(device_, image_view->image_view, nullptr);
                    ORION_CORE_LOG_INFO("Destroyed VkImageView {}", (void*)image_view->image_view);
                } else {
                    ORION_CORE_LOG_WARN("Attempting to destroy RHIImageView ({}) which not a valid Vulkan handle", handle.value);
                }
            }

            RHIImage get_swapchain_image_api(RHISwapchain swapchain, std::uint32_t image_idx) override
            {
                if (const auto* vulkan_swapchain = resources_.swapchains.get(swapchain.value)) {
                    if (image_idx < vulkan_swapchain->images.size()) {
                        return vulkan_swapchain->images[image_idx];
                    } else {
                        ORION_CORE_LOG_ERROR("Cannot get image {} from VkSwapchainKHR {} (image_count = {})",
                                             image_idx, (void*)vulkan_swapchain->swapchain, vulkan_swapchain->images.size());
                        return RHIImage::invalid();
                    }
                } else {
                    ORION_CORE_LOG_ERROR("Cannot get images from invalid RHISwapchain {}", swapchain.value);
                    return RHIImage::invalid();
                }
            }

            std::uint32_t acquire_swapchain_image_api(RHISwapchain swapchain, RHISemaphore semaphore, RHIFence fence) override
            {
                auto* vulkan_swapchain = resources_.swapchains.get(swapchain.value);
                if (!vulkan_swapchain) {
                    ORION_CORE_LOG_ERROR("Cannot acquire image for invalid Vulkan RHISwapchain {}", swapchain.value);
                    return UINT32_MAX;
                }

                VkSemaphore vk_semaphore = VK_NULL_HANDLE;
                if (semaphore.is_valid()) {
                    if (const auto* vulkan_semaphore = resources_.semaphores.get(semaphore.value)) {
                        vk_semaphore = *vulkan_semaphore;
                    } else {
                        ORION_CORE_LOG_ERROR("Invalid Vulkan RHISemaphore {}", semaphore.value);
                        return UINT32_MAX;
                    }
                }
                VkFence vk_fence = VK_NULL_HANDLE;
                if (fence.is_valid()) {
                    if (const auto* vulkan_fence = resources_.fences.get(fence.value)) {
                        vk_fence = *vulkan_fence;
                    } else {
                        ORION_CORE_LOG_ERROR("Invalid Vulkan RHIFence {}", fence.value);
                        return UINT32_MAX;
                    }
                }

                std::uint32_t image_index = 0;
                if (VkResult err = vkAcquireNextImageKHR(device_, vulkan_swapchain->swapchain, UINT64_MAX, vk_semaphore, vk_fence, &image_index)) {
                    ORION_CORE_LOG_ERROR("Failed to acquire image for VkSwapchainKHR {}: {}", (void*)vulkan_swapchain->swapchain, string_VkResult(err));
                    return UINT32_MAX;
                }
                vulkan_swapchain->current_image_index = image_index;
                return image_index;
            }

            void swapchain_present_api(RHISwapchain swapchain, std::span<const RHISemaphore> wait_semaphores) override
            {
                const auto* vulkan_swapchain = resources_.swapchains.get(swapchain.value);
                ORION_ASSERT(vulkan_swapchain != nullptr, "RHISwapchain must be a valid handle");

                std::vector<VkSemaphore> vk_wait_semaphores(wait_semaphores.size());
                std::ranges::transform(wait_semaphores, vk_wait_semaphores.begin(), [&](RHISemaphore semaphore) {
                    const auto vk_semaphore = resources_.semaphores.get(semaphore.value);
                    ORION_ASSERT(vk_semaphore != nullptr, "All RHISemaphore's must be valid handles");
                    return *vk_semaphore;
                });

                const auto present_info = VkPresentInfoKHR{
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .pNext = nullptr,
                    .waitSemaphoreCount = static_cast<std::uint32_t>(wait_semaphores.size()),
                    .pWaitSemaphores = vk_wait_semaphores.data(),
                    .swapchainCount = 1,
                    .pSwapchains = &vulkan_swapchain->swapchain,
                    .pImageIndices = &vulkan_swapchain->current_image_index,
                    .pResults = nullptr,
                };
                if (VkResult err = vkQueuePresentKHR(vulkan_swapchain->queue->vk_queue(), &present_info)) {
                    throw std::runtime_error(fmt::format("vkQueuePresentKHR failed: {}", string_VkResult(err)));
                }
            }

            bool wait_for_fences_api(std::span<const RHIFence> fences, bool wait_all, std::uint64_t timeout) override
            {
                std::vector<VkFence> vk_fences(fences.size());
                std::ranges::transform(fences, vk_fences.begin(), [&](RHIFence fence) {
                    const auto* vk_fence = resources_.fences.get(fence.value);
                    ORION_ASSERT(vk_fence != nullptr, "RHIFence must be a valid handle");
                    return *vk_fence;
                });
                if (VkResult err = vkWaitForFences(device_, static_cast<std::uint32_t>(fences.size()), vk_fences.data(), wait_all, timeout)) {
                    ORION_CORE_LOG_ERROR("vkWaitForFences failed: {}", string_VkResult(err));
                    return false;
                } else {
                    return true;
                }
            }

            bool reset_fences_api(std::span<const RHIFence> fences) override
            {
                std::vector<VkFence> vk_fences(fences.size());
                std::ranges::transform(fences, vk_fences.begin(), [&](RHIFence fence) {
                    const auto* vk_fence = resources_.fences.get(fence.value);
                    ORION_ASSERT(vk_fence != nullptr, "RHIFence must be a valid handle");
                    return *vk_fence;
                });
                if (VkResult err = vkResetFences(device_, static_cast<std::uint32_t>(fences.size()), vk_fences.data())) {
                    ORION_CORE_LOG_ERROR("vkResetFences failed: {}", string_VkResult(err));
                    return false;
                } else {
                    return true;
                }
            }

            VkResult create_shader_module(std::span<const std::byte> code, VkShaderModule* shader_module) const
            {
                const auto shader_info = VkShaderModuleCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .codeSize = static_cast<std::uint32_t>(code.size()),
                    .pCode = reinterpret_cast<const std::uint32_t*>(code.data()),
                };
                return vkCreateShaderModule(device_, &shader_info, nullptr, shader_module);
            }

            std::uint32_t get_queue_family_index(RHICommandQueueType type) const
            {
                switch (type) {
                    case RHICommandQueueType::Graphics:
                        return graphics_queue_.family;
                }
                unreachable();
            }

            VkInstance instance_;
            VkPhysicalDevice physical_device_;
            VkDevice device_;
            VulkanQueue graphics_queue_;
            VulkanResourceTable resources_;
        };

        class RHIVulkanInstance : public RHIInstance
        {
        public:
            RHIVulkanInstance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger)
                : instance_(instance)
                , debug_messenger_(debug_messenger)
            {
                ORION_ASSERT(instance != VK_NULL_HANDLE, "VkInstance must not be VK_NULL_HANDLE");
                ORION_ASSERT(debug_messenger != VK_NULL_HANDLE, "VkDebugUtilsMessengerEXT must not be VK_NULL_HANDLE");
            }

            ~RHIVulkanInstance() override
            {
                vkDestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
                ORION_CORE_LOG_INFO("Destroyed VkDebugUtilsMessengerEXT {}", (void*)debug_messenger_);
                vkDestroyInstance(instance_, nullptr);
                ORION_CORE_LOG_INFO("Destroyed VkInstance {}", (void*)instance_);
            }

        private:
            std::unique_ptr<RHIDevice> create_device_api() override
            {
                // Enumerate physical devices
                std::uint32_t physical_device_count = 0;
                if (VkResult result = vkEnumeratePhysicalDevices(instance_, &physical_device_count, nullptr); result != VK_SUCCESS) {
                    ORION_CORE_LOG_ERROR("Failed to enumerate Vulkan physical devices: {}", string_VkResult(result));
                    return nullptr;
                }
                if (physical_device_count == 0) {
                    ORION_CORE_LOG_ERROR("No valid Vulkan physical devices found");
                    return nullptr;
                }
                std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
                if (VkResult result = vkEnumeratePhysicalDevices(instance_, &physical_device_count, physical_devices.data()); result != VK_SUCCESS) {
                    ORION_CORE_LOG_ERROR("Failed to enumerate Vulkan physical devices: {}", string_VkResult(result));
                    return nullptr;
                }

                // Get physical device properties
                std::vector<VkPhysicalDeviceProperties> physical_device_properties(physical_device_count);
                std::ranges::transform(physical_devices, physical_device_properties.begin(), [](VkPhysicalDevice physical_device) {
                    VkPhysicalDeviceProperties properties;
                    vkGetPhysicalDeviceProperties(physical_device, &properties);
                    ORION_CORE_LOG_DEBUG("Found VkPhysicalDevice ({}): {} - {}", (void*)physical_device, properties.deviceName, string_VkPhysicalDeviceType(properties.deviceType));
                    return properties;
                });

                // Select physical device
                // TODO: Allow proper control over physical device selection
                VkPhysicalDevice physical_device = physical_devices[0];
                ORION_CORE_LOG_INFO("Selected VkPhysicalDevice ({}): {}", (void*)physical_device, physical_device_properties[0].deviceName);

                // Enumerate queue families
                std::uint32_t queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
                if (queue_family_count == 0) {
                    ORION_CORE_LOG_ERROR("No queue families found!");
                    return nullptr;
                }
                std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
                for (std::uint32_t idx = 0; const auto& queue_family : queue_families) {
                    ORION_CORE_LOG_DEBUG("Queue family {}: {{ flags: {}, queue_count: {} }}", idx++, string_VkQueueFlags(queue_family.queueFlags), queue_family.queueCount);
                }

                // Find graphics queue
                std::uint32_t graphics_queue_family = UINT32_MAX;
                for (std::uint32_t idx = 0; idx < queue_family_count; ++idx) {
                    if (queue_families[idx].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        graphics_queue_family = idx;
                        break;
                    }
                }
                if (graphics_queue_family == UINT32_MAX) {
                    ORION_CORE_LOG_ERROR("Failed to find a Vulkan queue family with VK_QUEUE_GRAPHICS_BIT");
                    return nullptr;
                }

                // Queue create info
                const auto queue_priorities = std::array{1.0f};
                const auto queue_infos = std::array{
                    VkDeviceQueueCreateInfo{
                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = {},
                        .queueFamilyIndex = graphics_queue_family,
                        .queueCount = 1,
                        .pQueuePriorities = queue_priorities.data(),
                    },
                };

                // Enabled device extensions
                std::vector<const char*> enabled_extensions;
                enabled_extensions.push_back("VK_KHR_swapchain");

                // MoltenVK requires VK_KHR_portability_subset
#ifdef ORION_VULKAN_MVK
                enabled_extensions.push_back("VK_KHR_portability_subset");
#endif

                // Create the device
                // Enable dynamic rendering
                const auto dynamic_rendering_features = VkPhysicalDeviceDynamicRenderingFeatures{
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
                    .pNext = nullptr,
                    .dynamicRendering = VK_TRUE,
                };
                const auto device_info = VkDeviceCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .pNext = &dynamic_rendering_features,
                    .flags = {},
                    .queueCreateInfoCount = static_cast<std::uint32_t>(queue_infos.size()),
                    .pQueueCreateInfos = queue_infos.data(),
                    .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
                    .ppEnabledExtensionNames = enabled_extensions.data(),
                    .pEnabledFeatures = nullptr,
                };
                VkDevice device = VK_NULL_HANDLE;
                if (VkResult result = vkCreateDevice(physical_device, &device_info, nullptr, &device); result != VK_SUCCESS) {
                    ORION_CORE_LOG_ERROR("Failed to create a Vulkan device: {}", string_VkResult(result));
                    return nullptr;
                }
                ORION_CORE_LOG_DEBUG("Created VkDevice {}", (void*)device);

                // Retrieve created queues
                VulkanQueue graphics_queue = {.family = graphics_queue_family};
                vkGetDeviceQueue(device, graphics_queue_family, 0, &graphics_queue.queue);
                ORION_CORE_LOG_DEBUG("Got VkQueue (graphics) {}", (void*)graphics_queue.queue);

                return std::make_unique<RHIVulkanDevice>(instance_, physical_device, device, graphics_queue);
            }

            VkInstance instance_;
            VkDebugUtilsMessengerEXT debug_messenger_;
        };
    } // namespace

    std::unique_ptr<RHIInstance> rhi_vulkan_create_instance()
    {
        if (volkInitialize() != VK_SUCCESS) {
            ORION_CORE_LOG_ERROR("Failed to initialize volk. Vulkan may not be installed on your system");
            return nullptr;
        }
        glfwInitVulkanLoader(vkGetInstanceProcAddr);

        // Instance create flags
        VkInstanceCreateFlags instance_flags = {};

        // Enabled instance layers
        std::vector<const char*> enabled_layers;
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");

        // Enabled instance extensions
        std::vector<const char*> enabled_extensions;
        enabled_extensions.push_back("VK_EXT_debug_utils");

        // MoltenVK requires VK_KHR_portability_enumeration & VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#ifdef ORION_VULKAN_MVK
        enabled_extensions.push_back("VK_KHR_portability_enumeration");
        instance_flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        // Add GLFW required extensions
        std::uint32_t glfw_extension_count;
        const char** extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        for (std::uint32_t i = 0; i < glfw_extension_count; ++i) {
            enabled_extensions.push_back(extensions[i]);
        }

        const auto app_info = VkApplicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Orion",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Orion",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = vulkan_api_version,
        };
        const auto instance_info = VkInstanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = instance_flags,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = static_cast<std::uint32_t>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
        };
        VkInstance instance = VK_NULL_HANDLE;
        if (VkResult result = vkCreateInstance(&instance_info, nullptr, &instance); result != VK_SUCCESS) {
            ORION_CORE_LOG_ERROR("Failed to create Vulkan instance: {}", string_VkResult(result));
            return nullptr;
        }
        volkLoadInstance(instance);
        ORION_CORE_LOG_INFO("Created VkInstance {}", (void*)instance);

        const auto debug_messenger_info = VkDebugUtilsMessengerCreateInfoEXT{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = {},
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = vulkan_debug_callback,
        };
        VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
        if (VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &debug_messenger_info, nullptr, &debug_messenger); result != VK_SUCCESS) {
            ORION_CORE_LOG_ERROR("Failed to created Vulkan debug messenger: {}", string_VkResult(result));
            return nullptr;
        }
        ORION_CORE_LOG_INFO("Created VkDebugUtilsMessengerEXT {}", (void*)debug_messenger);

        return std::make_unique<RHIVulkanInstance>(instance, debug_messenger);
    }
} // namespace orion
