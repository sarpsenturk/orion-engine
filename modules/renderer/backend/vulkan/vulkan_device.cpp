#include "vulkan_device.h"

#include "vulkan_conversion.h"
#include "vulkan_platform.h"

#include <numeric>                     // std::accumulate
#include <orion-utils/assertion.h>     // ORION_EXPECTS
#include <orion-utils/static_vector.h> // static_vector
#include <spdlog/spdlog.h>             // SPDLOG_*
#include <utility>                     // std::exchange

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

    VulkanDevice::VulkanDevice(spdlog::logger* logger, VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues)
        : RenderDevice(logger)
        , instance_(instance)
        , physical_device_(physical_device)
        , device_(std::move(device))
        , allocator_(create_vma_allocator(instance, physical_device, device_.get()))
        , queues_(queues)
    {
        // Create command pool for each queue
        auto create_pool_for_queue = [this](std::uint32_t queue_family) {
            if (!command_pools_.contains(queue_family)) {
                command_pools_.insert(std::make_pair(queue_family, create_vk_command_pool(device_.get(), queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)));
            }
        };

        create_pool_for_queue(queues_.graphics.index);
        create_pool_for_queue(queues_.transfer.index);
    }

    VulkanDevice::~VulkanDevice()
    {
        vkDeviceWaitIdle(device_.get());
    }

    VkCommandPool VulkanDevice::get_command_pool(CommandQueueType queue_type) const
    {
        switch (queue_type) {
            case CommandQueueType::Graphics:
                return graphics_command_pool();
            case CommandQueueType::Transfer:
                return transfer_command_pool();
            case CommandQueueType::Compute:
                ORION_ASSERT(!"Vulkan API doesn't currently support compute operations");
                break;
            case CommandQueueType::Any:
                break;
        }
        return VK_NULL_HANDLE;
    }

    VkQueue VulkanDevice::get_queue(CommandQueueType queue_type) const
    {
        switch (queue_type) {
            case CommandQueueType::Transfer:
                return transfer_queue();
            case CommandQueueType::Compute:
                return compute_queue();
            case CommandQueueType::Graphics:
                [[fallthrough]];
            case CommandQueueType::Any:
                return graphics_queue();
        }
        return VK_NULL_HANDLE;
    }

    SwapchainHandle VulkanDevice::create_swapchain_api(const Window& window, const SwapchainDesc& desc)
    {
        // Create the surface
        auto surface = create_surface(instance_, window);

        // Chose present mode TODO: Allow user to select this
        const auto present_mode = VK_PRESENT_MODE_FIFO_KHR;

        // Get the old swapchain
        const VkFormat vk_format = to_vulkan_type(desc.image_format);

        // Create the swapchain
        const SwapchainCreateInfo swapchain_info{
            .surface = surface.get(),
            .image_count = desc.image_count,
            .image_format = to_vulkan_type(desc.image_format),
            .image_size = to_vulkan_extent(desc.image_size),
            .present_mode = present_mode,
        };
        auto swapchain = create_vk_swapchain(device_.get(), physical_device_, swapchain_info);

        // Create render pass
        auto render_pass = [vk_format, device = device_.get()]() {
            const std::array attachments{
                VkAttachmentDescription{
                    .flags = 0,
                    .format = vk_format,
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
        }();

        // Create image view for each image and framebuffer for each image view
        std::vector<UniqueVkImageView> image_views;
        std::vector<UniqueVkFramebuffer> framebuffers;
        {
            // Acquire swapchain images
            auto swapchain_images = get_swapchain_images(device_.get(), swapchain.get());

            image_views.reserve(swapchain_images.size());
            framebuffers.reserve(swapchain_images.size());
            for (VkImage image : swapchain_images) {
                // Create image view
                VkImageView image_view = VK_NULL_HANDLE;
                {
                    image_views.push_back(create_vk_image_view(device_.get(), image, VK_IMAGE_VIEW_TYPE_2D, vk_format));
                    image_view = image_views.back().get();
                }

                // Create framebuffer
                framebuffers.emplace_back(create_vk_framebuffer(device_.get(), render_pass.get(), desc.image_size, {&image_view, 1}));
            }
        }

        auto handle = SwapchainHandle::generate();
        swapchains_.insert(std::make_pair(handle, VulkanSwapchain{
                                                      std::move(surface),
                                                      std::move(swapchain),
                                                      std::move(render_pass),
                                                      create_vk_semaphore(device_.get()),
                                                      std::move(image_views),
                                                      std::move(framebuffers),
                                                  }));
        return handle;
    }

    ShaderModuleHandle VulkanDevice::create_shader_module_api(const ShaderModuleDesc& desc)
    {
        // Convert to std::byte data to uint32_t
        std::vector<std::uint32_t> spirv(desc.byte_code.size_bytes() / sizeof(std::uint32_t));
        std::memcpy(spirv.data(), desc.byte_code.data(), desc.byte_code.size_bytes());

        auto handle = ShaderModuleHandle::generate();
        shader_modules_.insert(std::make_pair(handle, create_vk_shader_module(device_.get(), spirv)));
        return handle;
    }

    PipelineHandle VulkanDevice::create_graphics_pipeline_api(const GraphicsPipelineDesc& desc)
    {
        // Create VkPipelineLayout
        auto pipeline_layout = create_vk_pipeline_layout(device_.get(), {}, {});

        // Convert to VkPipelineShaderStageCreateInfo
        ORION_EXPECTS(desc.shaders.size() <= UINT32_MAX);
        const auto vk_stages = [stages = desc.shaders, this]() {
            std::vector<VkPipelineShaderStageCreateInfo> vk_stages;
            vk_stages.reserve(stages.size());
            for (const auto& stage : stages) {
                vk_stages.push_back({
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .stage = to_vulkan_type(stage.type),
                    .module = find_shader(stage.module.handle()),
                    .pName = stage.entry_point,
                    .pSpecializationInfo = nullptr,
                });
            }
            return vk_stages;
        }();

        // Convert to VkVertexInputBindingDescription
        const auto vk_input_bindings = [&vertex_bindings = desc.vertex_bindings]() {
            std::vector<VkVertexInputBindingDescription> vk_input_bindings;
            vk_input_bindings.reserve(vertex_bindings.size());
            for (std::uint32_t index = 0; const auto& binding : vertex_bindings) {
                vk_input_bindings.push_back({
                    .binding = index++,
                    .stride = binding.stride(),
                    .inputRate = to_vulkan_type(binding.input_rate()),
                });
            }
            return vk_input_bindings;
        }();

        // Convert to VkVertexInputAttributeDescription
        const auto vk_attribute_descriptions = [&vertex_bindings = desc.vertex_bindings]() {
            std::vector<VkVertexInputAttributeDescription> vk_attribute_descriptions;
            vk_attribute_descriptions.reserve(std::accumulate(vertex_bindings.begin(), vertex_bindings.end(), 0u, [](auto acc, const auto& binding) {
                return acc + static_cast<std::uint32_t>(binding.attributes().size());
            }));
            for (std::uint32_t binding_index = 0; const auto& binding : vertex_bindings) {
                for (std::uint32_t attr_index = 0; const auto& attribute : binding.attributes()) {
                    vk_attribute_descriptions.push_back({
                        .location = attr_index++,
                        .binding = binding_index,
                        .format = to_vulkan_type(attribute.format),
                        .offset = attribute.offset,
                    });
                }
                ++binding_index;
            }
            return vk_attribute_descriptions;
        }();

        // Create VkPipelineVertexInputStateCreateInfo
        const auto vk_input_state = [&vk_input_bindings, &vk_attribute_descriptions]() {
            return VkPipelineVertexInputStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .vertexBindingDescriptionCount = static_cast<std::uint32_t>(vk_input_bindings.size()),
                .pVertexBindingDescriptions = vk_input_bindings.data(),
                .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vk_attribute_descriptions.size()),
                .pVertexAttributeDescriptions = vk_attribute_descriptions.data(),
            };
        }();

        // Convert to VkPipelineInputAssemblyStateCreateInfo
        const auto vk_input_assembly = [input_assembly = desc.input_assembly]() {
            return VkPipelineInputAssemblyStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .topology = to_vulkan_type(input_assembly.topology),
                .primitiveRestartEnable = VK_FALSE,
            };
        }();

        // Create VkPipelineViewportStateCreateInfo
        const auto vk_viewport = []() {
            return VkPipelineViewportStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .viewportCount = 1,
                .scissorCount = 1,
            };
        }();

        // Convert to VkPipelineRasterizationStateCreateInfo
        const auto vk_rasterization = [rasterization = desc.rasterization]() {
            return VkPipelineRasterizationStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = nullptr,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = to_vulkan_type(rasterization.fill_mode),
                .cullMode = to_vulkan_type(rasterization.cull_mode),
                .frontFace = to_vulkan_type(rasterization.front_face),
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.f,
                .depthBiasClamp = 0.f,
                .depthBiasSlopeFactor = 0.f,
                .lineWidth = 1.f,
            };
        }();

        // Convert to VkPipelineMultisampleStateCreateInfo
        // Fixed for now. No multisampling support
        const auto vk_multisample = []() {
            return VkPipelineMultisampleStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.f,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE,
            };
        }();

        // Create VkPipelineColorBlendStateCreateInfo
        const auto vk_color_blend = []() {
            static const VkPipelineColorBlendAttachmentState attachment{
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };
            return VkPipelineColorBlendStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &attachment,
                .blendConstants = {0.f, 0.f, 0.f, 0.f},
            };
        }();

        // Create VkPipelineDynamicStateCreateInfo
        const auto vk_dynamic_state = []() {
            static const std::array dynamic_state{
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
            };
            return VkPipelineDynamicStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .dynamicStateCount = static_cast<std::uint32_t>(dynamic_state.size()),
                .pDynamicStates = dynamic_state.data(),
            };
        }();

        // Find associated render pass
        VkRenderPass render_pass = find_render_pass(desc.render_target);

        const VkGraphicsPipelineCreateInfo pipeline_info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = static_cast<std::uint32_t>(vk_stages.size()),
            .pStages = vk_stages.data(),
            .pVertexInputState = &vk_input_state,
            .pInputAssemblyState = &vk_input_assembly,
            .pTessellationState = nullptr,
            .pViewportState = &vk_viewport,
            .pRasterizationState = &vk_rasterization,
            .pMultisampleState = &vk_multisample,
            .pDepthStencilState = nullptr, // No depth stencil support yet
            .pColorBlendState = &vk_color_blend,
            .pDynamicState = &vk_dynamic_state,
            .layout = pipeline_layout.get(),
            .renderPass = render_pass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };

        const auto handle = PipelineHandle::generate();
        pipelines_.insert(std::make_pair(handle, VulkanPipeline{std::move(pipeline_layout), create_vk_graphics_pipeline(device_.get(), pipeline_info)}));
        return handle;
    }

    GPUBufferHandle VulkanDevice::create_buffer_api(const GPUBufferDesc& desc)
    {
        // Check if  buffer will be used for transfer ops
        const bool transfer_src = to_bool(desc.usage & GPUBufferUsageFlags::TransferSrc);
        const bool transfer_dst = to_bool(desc.usage & GPUBufferUsageFlags::TransferDst);

        // Find set of queue families to be used
        const auto queue_indices = [transfer_src, transfer_dst, this]() {
            static_vector<std::uint32_t, 2> queue_indices;
            queue_indices.push_back(queues_.graphics.index);
            if ((transfer_src || transfer_dst) && transfer_requires_concurrent(queues_.graphics.index)) {
                queue_indices.push_back(queues_.transfer.index);
            }
            return queue_indices;
        }();

        const auto handle = GPUBufferHandle::generate();
        const BufferCreateInfo create_info{
            .size = desc.size,
            .usage = to_vulkan_type(desc.usage),
            .queue_indices = {queue_indices.begin(), queue_indices.end()},
            .host_visible = desc.host_visible,
        };
        buffers_.insert(std::make_pair(handle, VulkanBuffer::create(allocator_.get(), create_info)));
        return handle;
    }

    CommandBufferHandle VulkanDevice::create_command_buffer_api(const CommandBufferDesc& desc)
    {
        // Find command pool based on queue type
        auto command_pool = get_command_pool(desc.queue_type);

        const auto handle = CommandBufferHandle::generate();
        command_buffers_.insert(std::make_pair(handle, allocate_vk_command_buffer(device_.get(), command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY)));
        return handle;
    }

    VkShaderModule VulkanDevice::find_shader(ShaderModuleHandle shader_module_handle) const
    {
        return shader_modules_.at(shader_module_handle).get();
    }

    VulkanSwapchain& VulkanDevice::find_swapchain(SwapchainHandle swapchain_handle)
    {
        return swapchains_.at(swapchain_handle);
    }

    VulkanBuffer& VulkanDevice::find_buffer(GPUBufferHandle buffer_handle)
    {
        return buffers_.at(buffer_handle);
    }

    VkRenderPass VulkanDevice::find_render_pass(RenderTargetHandle render_target_handle)
    {
        auto find_render_pass = [this](auto&& arg) -> VkRenderPass {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, SwapchainHandle>) {
                return find_swapchain(arg).render_pass();
            } else {
                return VK_NULL_HANDLE;
            }
        };
        return std::visit(find_render_pass, render_target_handle);
    }

    FindFramebufferResult VulkanDevice::find_framebuffer(RenderTargetHandle render_target_handle)
    {
        auto find_framebuffer = [this](auto&& arg) -> FindFramebufferResult {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, SwapchainHandle>) {
                auto& swapchain = find_swapchain(arg);
                VkSemaphore image_available = swapchain.image_semaphore();
                return {.framebuffer = swapchain.framebuffer(swapchain.next_image_index()), .available_semaphore = image_available};
            } else {
                return {};
            }
        };
        return std::visit(find_framebuffer, render_target_handle);
    }

    VkPipeline VulkanDevice::find_pipeline(PipelineHandle pipeline_handle) const
    {
        return pipelines_.at(pipeline_handle).pipeline();
    }

    VkCommandBuffer VulkanDevice::find_command_buffer(CommandBufferHandle command_buffer_handle) const
    {
        return command_buffers_.at(command_buffer_handle).get();
    }

    VulkanSubmission& VulkanDevice::find_submission(SubmissionHandle submission_handle)
    {
        return submissions_.at(submission_handle);
    }

    VkFence VulkanDevice::find_fence(SubmissionHandle submission_handle) const
    {
        return submissions_.at(submission_handle).fence.get();
    }

    VkSemaphore VulkanDevice::find_semaphore(SubmissionHandle submission_handle) const
    {
        return submissions_.at(submission_handle).semaphore.get();
    }

    void VulkanDevice::destroy_api(SwapchainHandle swapchain_handle)
    {
        swapchains_.erase(swapchain_handle);
    }

    void VulkanDevice::destroy_api(ShaderModuleHandle shader_module_handle)
    {
        shader_modules_.erase(shader_module_handle);
    }

    void VulkanDevice::destroy_api(PipelineHandle graphics_pipeline_handle)
    {
        pipelines_.erase(graphics_pipeline_handle);
    }

    void VulkanDevice::destroy_api(GPUBufferHandle buffer_handle)
    {
        buffers_.erase(buffer_handle);
    }

    void VulkanDevice::destroy_api(CommandBufferHandle command_buffer_handle)
    {
        command_buffers_.erase(command_buffer_handle);
    }

    void VulkanDevice::destroy_api(SubmissionHandle submission_handle)
    {
        submissions_.erase(submission_handle);
    }

    void* VulkanDevice::map_api(GPUBufferHandle buffer_handle)
    {
        const auto& vk_buffer = find_buffer(buffer_handle);
        void* ptr = nullptr;
        vk_result_check(vmaMapMemory(allocator_.get(), vk_buffer.allocation(), &ptr));
        SPDLOG_LOGGER_TRACE(logger(), "Mapped VkBuffer {} at memory address {}", fmt::ptr(vk_buffer.buffer()), fmt::ptr(ptr));
        return ptr;
    }

    void VulkanDevice::unmap_api(GPUBufferHandle buffer_handle)
    {
        vmaUnmapMemory(allocator_.get(), find_buffer(buffer_handle).allocation());
    }

    SubmissionHandle VulkanDevice::submit_api(const CommandBuffer& command_buffer, SubmissionHandle existing)
    {
        // Find and reset command buffer
        VkCommandBuffer vk_command_buffer = find_command_buffer(command_buffer.handle());
        vkResetCommandBuffer(vk_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        // Begin command buffer recording
        const auto begin_flags = [queue_type = command_buffer.queue_type()]() -> VkCommandBufferUsageFlags {
            if (queue_type == CommandQueueType::Transfer) {
                return VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            }
            return {};
        }();
        const VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = begin_flags,
            .pInheritanceInfo = nullptr,
        };
        vk_result_check(vkBeginCommandBuffer(vk_command_buffer, &begin_info));

        // Find existing or create new submission
        SubmissionHandle submission_handle = existing;
        auto& submission = [this, &submission_handle]() -> VulkanSubmission& {
            if (submission_handle.is_valid()) {
                return find_submission(submission_handle);
            }
            submission_handle = SubmissionHandle::generate();
            auto [iter, _] =
                submissions_.insert(std::make_pair(submission_handle, VulkanSubmission{.fence = create_vk_fence(device_.get()), .semaphore = create_vk_semaphore(device_.get())}));
            return iter->second;
        }();

        // Compile orion commands to vulkan commands
        compile_commands(vk_command_buffer, command_buffer.commands(), submission);

        // End command buffer recording
        vk_result_check(vkEndCommandBuffer(vk_command_buffer));

        // Submit command buffer
        const auto& wait_semaphores = submission.wait_semaphores;
        VkSemaphore signal_semaphore = submission.semaphore.get();
        VkFence signal_fence = submission.fence.get();
        const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        const VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<std::uint32_t>(wait_semaphores.size()),
            .pWaitSemaphores = wait_semaphores.data(),
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &vk_command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &signal_semaphore,
        };
        vk_result_check(vkQueueSubmit(get_queue(command_buffer.queue_type()), 1, &submit_info, signal_fence));
        return submission_handle;
    }

    void VulkanDevice::wait_api(SubmissionHandle submission_handle)
    {
        // We allow for invalid submission handles to handle first frame submissions
        if (VkFence fence = find_fence(submission_handle)) {
            vk_result_check(vkWaitForFences(device_.get(), 1, &fence, VK_TRUE, UINT64_MAX));
            vkResetFences(device_.get(), 1, &fence);
        }
    }

    void VulkanDevice::present_api(SwapchainHandle swapchain_handle, SubmissionHandle wait)
    {
        // Find presentation queue
        VkQueue present_queue = get_queue(CommandQueueType::Graphics);

        // Find swapchain and resources
        auto& swapchain = find_swapchain(swapchain_handle);
        VkSwapchainKHR vk_swapchain = swapchain.swapchain();
        const std::uint32_t image_index = swapchain.current_image_index();

        // Find submission (if any) to wait for
        VkSemaphore semaphore = [this, wait]() -> VkSemaphore {
            if (wait.is_valid()) {
                return find_semaphore(wait);
            }
            return VK_NULL_HANDLE;
        }();

        // Present image
        const VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &semaphore,
            .swapchainCount = 1,
            .pSwapchains = &vk_swapchain,
            .pImageIndices = &image_index,
            .pResults = nullptr,
        };
        vk_result_check(vkQueuePresentKHR(present_queue, &present_info));
    }

    void VulkanDevice::compile_commands(VkCommandBuffer command_buffer, const std::vector<CommandPacket>& commands, VulkanSubmission& submission)
    {
        submission.wait_semaphores.clear();
        for (auto command : commands) {
            switch (command.command_type) {
                case CommandType::BufferCopy:
                    cmd_buffer_copy(command_buffer, command.data);
                    break;
                case CommandType::BeginFrame:
                    if (VkSemaphore semaphore = cmd_begin_frame(command_buffer, command.data)) {
                        submission.wait_semaphores.push_back(semaphore);
                    }
                    break;
                case CommandType::EndFrame:
                    cmd_end_frame(command_buffer, command.data);
                    break;
                case CommandType::Draw:
                    cmd_draw(command_buffer, command.data);
                    break;
            }
        }
    }

    void VulkanDevice::cmd_buffer_copy(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* const cmd_buffer_copy = static_cast<const CmdBufferCopy*>(data);
        VkBuffer src_buffer = find_buffer(cmd_buffer_copy->src).buffer();
        VkBuffer dst_buffer = find_buffer(cmd_buffer_copy->dst).buffer();
        const std::array buffer_copy{
            VkBufferCopy{
                .srcOffset = 0,
                .dstOffset = 0,
                .size = cmd_buffer_copy->size,
            },
        };
        vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, buffer_copy.data());
    }

    VkSemaphore VulkanDevice::cmd_begin_frame(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* const cmd_begin_frame = static_cast<const CmdBeginFrame*>(data);
        // Clear values
        const VkClearColorValue clear_color = {cmd_begin_frame->clear_color.x(), cmd_begin_frame->clear_color.y(), cmd_begin_frame->clear_color.z(), cmd_begin_frame->clear_color.w()};
        const std::array clear_values{VkClearValue{.color = clear_color}};

        // Find framebuffer
        const auto framebuffer = find_framebuffer(cmd_begin_frame->render_target);
        const VkRenderPassBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = find_render_pass(cmd_begin_frame->render_target),
            .framebuffer = framebuffer.framebuffer,
            .renderArea = {.offset = {}, .extent = to_vulkan_extent(cmd_begin_frame->render_area)},
            .clearValueCount = static_cast<std::uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data(),
        };
        vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
        return framebuffer.available_semaphore;
    }

    void VulkanDevice::cmd_end_frame(VkCommandBuffer command_buffer, const void* data)
    {
        (void)data;
        vkCmdEndRenderPass(command_buffer);
    }

    void VulkanDevice::cmd_draw(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* const cmd_draw = static_cast<const CmdDraw*>(data);

        // Bind graphics pipeline
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, find_pipeline(cmd_draw->graphics_pipeline));

        // Bind vertex buffer
        VkBuffer vertex_buffer = find_buffer(cmd_draw->vertex_buffer).buffer();
        const std::array offsets{VkDeviceSize{0}};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offsets.data());

        // Set viewport and scissor
        const VkViewport viewport = to_vulkan_type(cmd_draw->viewport);
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        const VkRect2D scissor{.offset = {0, 0}, .extent = {static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height)}};
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        // Issue draw command
        vkCmdDraw(command_buffer, cmd_draw->vertex_count, 1, cmd_draw->first_vertex, 0);
    }
} // namespace orion::vulkan
