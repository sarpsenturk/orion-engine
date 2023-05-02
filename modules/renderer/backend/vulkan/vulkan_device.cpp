#include "vulkan_device.h"

#include "vulkan_conversion.h"
#include "vulkan_platform.h"
#include "vulkan_render_context.h"

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
        , allocator_(create_allocator())
        , queues_(queues)
    {
        // Create command pool for each queue
        auto create_pool_for_queue = [this](std::uint32_t queue_family) {
            if (!command_pools_.contains(queue_family)) {
                command_pools_.insert(std::make_pair(queue_family, create_command_pool(device_.get(), queue_family)));
            }
        };

        create_pool_for_queue(queues_.graphics.index);
        create_pool_for_queue(queues_.transfer.index);
    }

    UniqueVmaAllocator VulkanDevice::create_allocator()
    {
        const VmaAllocatorCreateInfo allocator_info{
            .flags = 0,
            .physicalDevice = physical_device_,
            .device = device_.get(),
            .preferredLargeHeapBlockSize = 0,
            .pHeapSizeLimit = nullptr,
            .pVulkanFunctions = nullptr,
            .instance = instance_,
            .vulkanApiVersion = vulkan_api_version,
            .pTypeExternalMemoryHandleTypes = nullptr,
        };
        VmaAllocator allocator = VK_NULL_HANDLE;
        vk_result_check(vmaCreateAllocator(&allocator_info, &allocator));
        SPDLOG_LOGGER_DEBUG(logger(), "Created VmaAllocator {}", fmt::ptr(allocator));
        return {allocator, {}};
    }

    VkCommandPool VulkanDevice::graphics_command_pool() const
    {
        return command_pools_.at(queues_.graphics.index).get();
    }

    VkCommandPool VulkanDevice::transfer_command_pool() const
    {
        return command_pools_.at(queues_.transfer.index).get();
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

    std::unique_ptr<RenderContext> VulkanDevice::create_render_context()
    {
        const VkCommandPoolCreateInfo command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queues_.graphics.index,
        };
        VkCommandPool command_pool = VK_NULL_HANDLE;
        vk_result_check(vkCreateCommandPool(device_.get(), &command_pool_create_info, alloc_callbacks(), &command_pool));
        return std::make_unique<VulkanRenderContext>(UniqueVkCommandPool{command_pool, CommandPoolDeleter{device_.get()}});
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
            vk_result_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface.get(), &surface_capabilities));
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
                .surface = surface.get(),
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
            vk_result_check(vkCreateSwapchainKHR(device_.get(), &info, alloc_callbacks(), &swapchain));
            SPDLOG_LOGGER_DEBUG(logger(), "Created VkSwapchainKHR {}", fmt::ptr(swapchain));
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

            const VkSubpassDependency subpass_dependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dependencyFlags = 0,
            };

            const VkRenderPassCreateInfo render_pass_info{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .attachmentCount = 1,
                .pAttachments = &color_attachment,
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 1,
                .pDependencies = &subpass_dependency,
            };

            vk_result_check(vkCreateRenderPass(device_.get(), &render_pass_info, alloc_callbacks(), &render_pass));
            SPDLOG_LOGGER_TRACE(logger(), "Created render pass for swapchain", fmt::ptr(render_pass));
        }

        // Create image view for each image and framebuffer for each image view
        std::vector<UniqueVkImageView> image_views;
        std::vector<UniqueVkFramebuffer> framebuffers;
        {
            // Acquire swapchain images
            auto swapchain_images = get_swapchain_images(device_.get(), swapchain);

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
                    vk_result_check(vkCreateImageView(device_.get(), &image_view_info, alloc_callbacks(), &image_view));
                    image_views.emplace_back(image_view, ImageViewDeleter{device_.get()});
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
                    vk_result_check(vkCreateFramebuffer(device_.get(), &framebuffer_info, alloc_callbacks(), &framebuffer));
                    framebuffers.emplace_back(framebuffer, FramebufferDeleter{device_.get()});
                }
            }
            SPDLOG_LOGGER_TRACE(logger(), "Created {} image view(s) and framebuffer(s) for swapchain", swapchain_images.size());
        }

        auto handle = existing.is_valid() ? existing : SwapchainHandle::generate();
        swapchains_.insert_or_assign(handle, VulkanSwapchain{
                                                 std::move(surface),
                                                 UniqueVkSwapchainKHR{swapchain, SwapchainDeleter{device_.get()}},
                                                 UniqueVkRenderPass{render_pass, RenderPassDeleter{device_.get()}},
                                                 create_semaphore(device_.get()),
                                                 std::move(image_views),
                                                 std::move(framebuffers),
                                             });
        return handle;
    }

    ShaderModuleHandle VulkanDevice::create_shader_module_api(const ShaderModuleDesc& desc, ShaderModuleHandle existing)
    {
        // Convert to std::byte data to uint32_t
        std::vector<std::uint32_t> spirv(desc.byte_code.size_bytes() / sizeof(std::uint32_t));
        std::memcpy(spirv.data(), desc.byte_code.data(), desc.byte_code.size_bytes());

        const VkShaderModuleCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = desc.byte_code.size_bytes(),
            .pCode = spirv.data(),
        };
        VkShaderModule shader_module = VK_NULL_HANDLE;
        vk_result_check(vkCreateShaderModule(device_.get(), &info, alloc_callbacks(), &shader_module));
        SPDLOG_LOGGER_DEBUG(logger(), "Created VkShaderModule {}", fmt::ptr(shader_module));

        auto handle = existing.is_valid() ? existing : ShaderModuleHandle::generate();
        shader_modules_.insert_or_assign(handle, UniqueVkShaderModule{shader_module, ShaderModuleDeleter{device_.get()}});
        return handle;
    }

    PipelineHandle VulkanDevice::create_graphics_pipeline_api(const GraphicsPipelineDesc& desc, PipelineHandle existing)
    {
        // Create VkPipelineLayout
        const auto vk_pipeline_layout = [device = device_.get()]() {
            const VkPipelineLayoutCreateInfo layout_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = 0,
                .pSetLayouts = nullptr,
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr,
            };
            VkPipelineLayout vk_pipeline_layout = VK_NULL_HANDLE;
            vk_result_check(vkCreatePipelineLayout(device, &layout_info, alloc_callbacks(), &vk_pipeline_layout));
            return vk_pipeline_layout;
        }();

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
            static VkPipelineColorBlendAttachmentState attachment{
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
                .logicOp = VK_LOGIC_OP_NO_OP,
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
        const auto render_pass = find_render_pass(desc.render_target);

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
            .layout = vk_pipeline_layout,
            .renderPass = render_pass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };
        VkPipeline vk_pipeline = VK_NULL_HANDLE;
        vk_result_check(vkCreateGraphicsPipelines(device_.get(), VK_NULL_HANDLE, 1, &pipeline_info, alloc_callbacks(), &vk_pipeline));
        SPDLOG_LOGGER_DEBUG(logger(), "Created VkPipeline (graphics) {}", fmt::ptr(vk_pipeline));

        const auto handle = existing.is_valid() ? existing : PipelineHandle::generate();
        pipelines_.insert_or_assign(handle, VulkanPipeline{
                                                UniqueVkPipelineLayout{vk_pipeline_layout, PipelineLayoutDeleter{device_.get()}},
                                                UniqueVkPipeline{vk_pipeline, PipelineDeleter{device_.get()}},
                                            });
        return handle;
    }

    GPUBufferHandle VulkanDevice::create_buffer_api(const GPUBufferDesc& desc, GPUBufferHandle existing)
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
        SPDLOG_LOGGER_TRACE(logger(), "Buffer access available to queue families: {}", queue_indices);

        const auto handle = existing.is_valid() ? existing : GPUBufferHandle::generate();
        const BufferCreateInfo create_info{
            .size = desc.size,
            .usage = to_vulkan_type(desc.usage),
            .queue_indices = {queue_indices.begin(), queue_indices.end()},
            .host_visible = desc.host_visible,
        };
        buffers_.insert_or_assign(handle, VulkanBuffer::create(allocator_.get(), create_info));
        return handle;
    }

    CommandBufferHandle VulkanDevice::create_command_buffer_api(const CommandBufferDesc& desc, CommandBufferHandle existing)
    {
        // Find command pool based on queue type
        auto command_pool = get_command_pool(desc.queue_type);

        // Allocate command buffer
        const VkCommandBufferAllocateInfo allocate_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        vk_result_check(vkAllocateCommandBuffers(device_.get(), &allocate_info, &command_buffer));

        const auto handle = existing.is_valid() ? existing : CommandBufferHandle::generate();
        command_buffers_.insert_or_assign(handle, UniqueVkCommandBuffer{command_buffer, CommandBufferDeleter{device_.get(), command_pool}});
        return handle;
    }

    VkShaderModule VulkanDevice::find_shader(ShaderModuleHandle shader_module_handle) const
    {
        return shader_modules_.at(shader_module_handle).get();
    }

    const VulkanSwapchain& VulkanDevice::find_swapchain(SwapchainHandle swapchain_handle) const
    {
        return swapchains_.at(swapchain_handle);
    }

    const VulkanBuffer& VulkanDevice::find_buffer(GPUBufferHandle buffer_handle) const
    {
        return buffers_.at(buffer_handle);
    }

    VkRenderPass VulkanDevice::find_render_pass(RenderTargetHandle render_target_handle) const
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

    VkPipeline VulkanDevice::find_pipeline(PipelineHandle pipeline_handle) const
    {
        return pipelines_.at(pipeline_handle).pipeline();
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
} // namespace orion::vulkan
