#include "orion/renderer/renderer.hpp"

#include "vulkan_impl.hpp"
#include <vulkan/vk_enum_string_helper.h>

#include "orion/log.hpp"
#include "orion/window.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>

namespace orion
{
    static constexpr auto frames_in_flight = 2;

    struct PerFrameData {
        VulkanCommandPool command_pool;

        VulkanSemaphore image_available_semaphore;
        VulkanSemaphore render_complete_semaphore;
    };

    struct Renderer::Impl {
        VulkanInstance vulkan_instance;
        VulkanDevice vulkan_device;
        VulkanSurface vulkan_surface;
        VulkanSwapchain vulkan_swapchain;

        std::uint64_t frame_count = 0;
        std::array<PerFrameData, frames_in_flight> frame_data;
        VulkanSemaphore frame_semaphore;

        Impl(
            VulkanInstance instance,
            VulkanDevice device,
            VulkanSurface surface,
            VulkanSwapchain swapchain,
            std::array<PerFrameData, frames_in_flight> frame_data,
            VulkanSemaphore frame_semaphore)
            : vulkan_instance(std::move(instance))
            , vulkan_device(std::move(device))
            , vulkan_surface(std::move(surface))
            , vulkan_swapchain(std::move(swapchain))
            , frame_data(std::move(frame_data))
            , frame_semaphore(std::move(frame_semaphore))
        {
        }

        ~Impl() { (void)vulkan_device.wait_idle(); }

        void render()
        {
            // Wait until previous render has finished
            const auto wait_value = (frame_count >= frames_in_flight) ? frame_count - (frames_in_flight - 1) : 0;
            if (!frame_semaphore.wait(wait_value, UINT64_MAX)) {
                throw std::runtime_error("vkWaitSemaphores() failed");
            }

            // Reset command buffers
            auto& fd = frame_data[frame_count % frames_in_flight];
            if (!fd.command_pool.reset()) {
                throw std::runtime_error("vkResetCommandPool() failed");
            }

            // Acquire swapchain image
            const auto image_index = vulkan_swapchain.acquire_next_image(fd.image_available_semaphore, UINT64_MAX);
            if (!image_index) {
                throw std::runtime_error("vkAcquireNextImageKHR failed. Swapchain recreation not supported yet");
            }
            VkImage swapchain_image = vulkan_swapchain.vk_images[*image_index];
            VkImageView swapchain_image_view = vulkan_swapchain.vk_image_views[*image_index];

            // Render frame
            //  Begin command buffer recording
            auto command_buffer = fd.command_pool.begin_command_buffer();
            if (!command_buffer) {
                throw std::runtime_error("vkBeginCommandBuffer failed");
            }

            //  Transition swapchain image to color attachment
            const auto to_color_attachment_barrier = VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                .srcAccessMask = VK_ACCESS_2_NONE,
                .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = swapchain_image,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            const auto to_color_attachment_info = VkDependencyInfo{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .pNext = nullptr,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &to_color_attachment_barrier,
            };
            vkCmdPipelineBarrier2(*command_buffer, &to_color_attachment_info);

            //  Begin render pass
            const auto clear_color = VkClearColorValue{{1.0f, 0.0f, 1.0f, 1.0f}};
            const auto color_attachment_info = VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = swapchain_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .resolveImageView = VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {clear_color},
            };
            const auto rendering_info = VkRenderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .pNext = nullptr,
                .flags = {},
                .renderArea = {
                    .extent = vulkan_swapchain.image_extent,
                },
                .layerCount = 1,
                .viewMask = 0,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color_attachment_info,
            };
            vkCmdBeginRendering(*command_buffer, &rendering_info);

            // Draw commands

            //  End render pass
            vkCmdEndRendering(*command_buffer);

            //  Transition swapchain image to present src
            const auto to_present_src_barrier = VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = swapchain_image,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            const auto to_present_src_info = VkDependencyInfo{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .pNext = nullptr,
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &to_present_src_barrier,
            };
            vkCmdPipelineBarrier2(*command_buffer, &to_present_src_info);

            //  End command buffer recording
            if (VkResult err = vkEndCommandBuffer(*command_buffer)) {
                ORION_RENDERER_LOG_ERROR("vkEndCommandBuffer() failed: {}", string_VkResult(err));
                throw std::runtime_error("vkEndCommandBuffer failed");
            }

            // Submit command buffer to queue
            const auto wait_semaphores = VkSemaphoreSubmitInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .pNext = nullptr,
                .semaphore = fd.image_available_semaphore.vk_semaphore,
                .value = 0, // ignored, binary semaphore
                .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .deviceIndex = 0,
            };
            const auto cb_submit_info = VkCommandBufferSubmitInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .pNext = nullptr,
                .commandBuffer = *command_buffer,
                .deviceMask = 0,
            };
            const auto signal_semaphores = std::array{
                VkSemaphoreSubmitInfo{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .pNext = nullptr,
                    .semaphore = fd.render_complete_semaphore.vk_semaphore,
                    .value = 0, // ignored, binary semaphore
                    .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                    .deviceIndex = 0,
                },
                VkSemaphoreSubmitInfo{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .pNext = nullptr,
                    .semaphore = frame_semaphore.vk_semaphore,
                    .value = ++frame_count, // Increment frame/submission count
                    .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                    .deviceIndex = 0,
                },
            };
            const auto submit_info = VkSubmitInfo2{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .pNext = nullptr,
                .flags = {},
                .waitSemaphoreInfoCount = 1,
                .pWaitSemaphoreInfos = &wait_semaphores,
                .commandBufferInfoCount = 1,
                .pCommandBufferInfos = &cb_submit_info,
                .signalSemaphoreInfoCount = 2,
                .pSignalSemaphoreInfos = signal_semaphores.data(),
            };
            if (VkResult err = vkQueueSubmit2(vulkan_device.graphics_queue, 1, &submit_info, VK_NULL_HANDLE)) {
                ORION_RENDERER_LOG_ERROR("vkQueueSubmit2() failed: {}", string_VkResult(err));
                throw std::runtime_error("vkQueueSubmit2 failed");
            }

            // Present swapchain image
            const auto present_info = VkPresentInfoKHR{
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &fd.render_complete_semaphore.vk_semaphore,
                .swapchainCount = 1,
                .pSwapchains = &vulkan_swapchain.vk_swapchain,
                .pImageIndices = &image_index.value(),
                .pResults = nullptr,
            };
            if (VkResult err = vkQueuePresentKHR(vulkan_device.graphics_queue, &present_info)) {
                ORION_RENDERER_LOG_ERROR("vkQueuePresentKHR() failed: {}", string_VkResult(err));
                throw std::runtime_error("vkQueuePresentKHR failed");
            }
        }
    };

    tl::expected<Renderer, std::string> Renderer::initialize(const RendererDesc& desc)
    {
        // Create vulkan instance
        auto vulkan_instance = VulkanInstance::create();
        if (!vulkan_instance) {
            return tl::unexpected("Failed to create Vulkan instance");
        }

        // Get list of available GPUs
        const auto physical_devices = vulkan_instance->enumerate_physical_devices();
        if (!physical_devices) {
            return tl::unexpected("Failed to enumerate physical devices");
        }

        // Create device
        // Use first available GPU for now
        // TODO: Allow user to select this
        VkPhysicalDevice physical_device = (*physical_devices)[0];
        auto vulkan_device = vulkan_instance->create_device(physical_device);
        if (!vulkan_device) {
            return tl::unexpected("Failed to create Vulkan device");
        }

        // Create window surface
        auto vulkan_surface = vulkan_device->create_surface(desc.window);
        if (!vulkan_surface) {
            return tl::unexpected("Failed to create Vulkan surface");
        }

        // Create swapchain
        auto vulkan_swapchain = vulkan_device->create_swapchain({
            .surface = *vulkan_surface,
            .requested_extent = {
                static_cast<std::uint32_t>(desc.window.width()),
                static_cast<std::uint32_t>(desc.window.height()),
            },
            .requested_image_count = 2,
            .requested_image_format = VK_FORMAT_B8G8R8A8_SRGB,
            .requested_present_mode = VK_PRESENT_MODE_FIFO_KHR,
        });
        if (!vulkan_swapchain) {
            return tl::unexpected("Failed to create Vulkan swapchain");
        }

        // Create per frame resources
        std::array<PerFrameData, frames_in_flight> frame_data;
        for (std::uint32_t i = 0; i < frames_in_flight; ++i) {
            auto command_pool = vulkan_device->create_command_pool({vulkan_device->graphics_queue_family, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT});
            if (!command_pool) {
                return tl::unexpected("Failed to create Vulkan command pool");
            }
            frame_data[i].command_pool = std::move(*command_pool);

            auto image_available_semaphore = vulkan_device->create_binary_semaphore();
            if (!image_available_semaphore) {
                return tl::unexpected("Failed to create Vulkan semaphpre");
            }
            frame_data[i].image_available_semaphore = std::move(*image_available_semaphore);
            auto render_complete_semaphore = vulkan_device->create_binary_semaphore();
            if (!render_complete_semaphore) {
                return tl::unexpected("Failed to create Vulkan semaphpre");
            }
            frame_data[i].render_complete_semaphore = std::move(*render_complete_semaphore);
        }

        // Create frame counter semaphore
        auto frame_semaphore = vulkan_device->create_timeline_semaphore(0);
        if (!frame_semaphore) {
            return tl::unexpected("Failed to create Vulkan semaphore");
        }

        return Renderer{std::make_unique<Impl>(
            std::move(*vulkan_instance),
            std::move(*vulkan_device),
            std::move(*vulkan_surface),
            std::move(*vulkan_swapchain),
            std::move(frame_data),
            std::move(*frame_semaphore))};
    }

    Renderer::Renderer(std::unique_ptr<Impl> impl)
        : impl_(std::move(impl))
    {
    }
    // Need to explicitly defaut here where Impl is defined
    Renderer::Renderer(Renderer&&) noexcept = default;
    Renderer& Renderer::operator=(Renderer&&) noexcept = default;
    Renderer::~Renderer() = default;

    void Renderer::render()
    {
        impl_->render();
    }
} // namespace orion
