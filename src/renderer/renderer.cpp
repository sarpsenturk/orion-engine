#include "orion/renderer/renderer.hpp"

#include "orion/renderer/render_graph.hpp"

#include "vulkan_impl.hpp"
#include <vulkan/vk_enum_string_helper.h>

#include "orion/log.hpp"
#include "orion/window.hpp"

#include "imgui_context.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

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

        RenderGraph render_graph;
    };

    struct Renderer::Impl {
        VulkanInstance vulkan_instance;
        VulkanDevice vulkan_device;
        VulkanSurface vulkan_surface;
        VulkanSwapchain vulkan_swapchain;

        std::uint64_t frame_count = 0;
        std::array<PerFrameData, frames_in_flight> frame_data;
        VulkanSemaphore frame_semaphore;

        ImGuiContextWrapper imgui_context;
        VkResult swapchain_status = VK_SUCCESS;

        Impl(
            VulkanInstance instance,
            VulkanDevice device,
            VulkanSurface surface,
            VulkanSwapchain swapchain,
            std::array<PerFrameData, frames_in_flight> frame_data,
            VulkanSemaphore frame_semaphore,
            ImGuiContextWrapper imgui_context)
            : vulkan_instance(std::move(instance))
            , vulkan_device(std::move(device))
            , vulkan_surface(std::move(surface))
            , vulkan_swapchain(std::move(swapchain))
            , frame_data(std::move(frame_data))
            , frame_semaphore(std::move(frame_semaphore))
            , imgui_context(std::move(imgui_context))
        {
        }

        ~Impl() { (void)vulkan_device.wait_idle(); }

        void new_frame()
        {
            // Start the Dear ImGui frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

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
                // Swapchain needs to be recreated, skip rendering this frame
                if (image_index.error() == VK_ERROR_OUT_OF_DATE_KHR) {
                    swapchain_status = image_index.error();
                    return;
                } else {
                    // Another, unrecoverable error occured
                    throw std::runtime_error(fmt::format("vkAcquireNextImageKHR() failed: {}", string_VkResult(image_index.error())));
                }
            }

            // Render frame
            //  Begin command buffer recording
            auto command_buffer = fd.command_pool.begin_command_buffer();
            if (!command_buffer) {
                throw std::runtime_error("vkBeginCommandBuffer failed");
            }

            // Reset render graph
            fd.render_graph.reset();

            // Import swapchain image to render graph
            auto swapchain_image = fd.render_graph.import_texture(
                vulkan_swapchain.vk_images[*image_index],
                vulkan_swapchain.vk_image_views[*image_index],
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            // Define empty clear pass
            fd.render_graph.add_pass("Clear", [&](RenderPassBuilder& builder) {
                swapchain_image = builder.write_texture(swapchain_image, TextureUsage::ColorAttachment);
                return [=](RenderPassContext& ctx) {
                    const auto clear_color = VkClearColorValue{{1.0f, 0.0f, 1.0f, 1.0f}};
                    const auto color_attachment = VkRenderingAttachmentInfo{
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = ctx.get_image_view(swapchain_image),
                        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .clearValue = {clear_color},
                    };
                    const auto rendering_info = VkRenderingInfo{
                        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                        .renderArea = {.extent = vulkan_swapchain.image_extent},
                        .layerCount = 1,
                        .colorAttachmentCount = 1,
                        .pColorAttachments = &color_attachment,
                    };
                    vkCmdBeginRendering(ctx.cmd(), &rendering_info);
                    vkCmdEndRendering(ctx.cmd());
                };
            });

            // Define imgui pass
            fd.render_graph.add_pass("ImGui", [&](RenderPassBuilder& builder) {
                swapchain_image = builder.write_texture(swapchain_image, TextureUsage::ColorAttachment);
                return [=](RenderPassContext& ctx) {
                    const auto color_attachment = VkRenderingAttachmentInfo{
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = ctx.get_image_view(swapchain_image),
                        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    };
                    const auto rendering_info = VkRenderingInfo{
                        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                        .renderArea = {.extent = vulkan_swapchain.image_extent},
                        .layerCount = 1,
                        .colorAttachmentCount = 1,
                        .pColorAttachments = &color_attachment,
                    };
                    vkCmdBeginRendering(ctx.cmd(), &rendering_info);
                    ImGui::Render();
                    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), ctx.cmd());
                    vkCmdEndRendering(ctx.cmd());
                };
            });

            // Compile & execute render graph
            fd.render_graph.compile();
            fd.render_graph.execute(*command_buffer);

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
            if (VkResult result = vkQueuePresentKHR(vulkan_device.graphics_queue, &present_info)) {
                // Swapchain needs to be recreated, it will be before next render
                if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
                    swapchain_status = result;
                    return;
                } else {
                    // Another, unrecoverable error occured
                    throw std::runtime_error(fmt::format("vkQueuePresentKHR failed", string_VkResult(result)));
                }
            }
        }

        [[nodiscard]] bool swapchain_out_of_date() const noexcept
        {
            return swapchain_status == VK_SUBOPTIMAL_KHR || swapchain_status == VK_ERROR_OUT_OF_DATE_KHR;
        }

        tl::expected<void, std::string> recreate_swapchain(int width, int height)
        {
            // Wait until device is idle to make sure swapchain is not in use
            (void)vulkan_device.wait_idle();

            // Create new swapchain
            auto new_swapchain = vulkan_device.create_swapchain({
                .surface = vulkan_surface,
                .requested_extent = {
                    .width = static_cast<std::uint32_t>(width),
                    .height = static_cast<std::uint32_t>(height),
                },
                .requested_image_count = vulkan_swapchain.image_count,
                .requested_image_format = vulkan_swapchain.image_format,
                .requested_present_mode = vulkan_swapchain.present_mode,
                .old_swapchain = vulkan_swapchain.vk_swapchain,
            });
            if (!new_swapchain) {
                return tl::unexpected(fmt::format("Failed to recreate swapchain: {}", string_VkResult(new_swapchain.error())));
            }

            // Reassign swapchain, destroy old swapchain & resources
            vulkan_swapchain = std::move(*new_swapchain);

            // Override ImGui MinImageCount
            ImGui_ImplVulkan_SetMinImageCount(vulkan_swapchain.image_count);

            // Reset swapchain status
            swapchain_status = VK_SUCCESS;

            return {};
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

        // Initialize imgui
        auto imgui_context = ImGuiContextWrapper::create({
            desc.window,
            *vulkan_device,
            *vulkan_swapchain,
        });
        if (!imgui_context) {
            return tl::unexpected(std::move(imgui_context.error()));
        }

        return Renderer{std::make_unique<Impl>(
            std::move(*vulkan_instance),
            std::move(*vulkan_device),
            std::move(*vulkan_surface),
            std::move(*vulkan_swapchain),
            std::move(frame_data),
            std::move(*frame_semaphore),
            std::move(*imgui_context))};
    }

    Renderer::Renderer(std::unique_ptr<Impl> impl)
        : impl_(std::move(impl))
    {
    }
    // Need to explicitly defaut here where Impl is defined
    Renderer::Renderer(Renderer&&) noexcept = default;
    Renderer& Renderer::operator=(Renderer&&) noexcept = default;
    Renderer::~Renderer() = default;

    void Renderer::new_frame()
    {
        impl_->new_frame();
    }

    void Renderer::render()
    {
        impl_->render();
    }

    bool Renderer::swapchain_out_of_date() const noexcept
    {
        return impl_->swapchain_out_of_date();
    }

    tl::expected<void, std::string> Renderer::recreate_swapchain(int width, int height)
    {
        return impl_->recreate_swapchain(width, height);
    }
} // namespace orion
