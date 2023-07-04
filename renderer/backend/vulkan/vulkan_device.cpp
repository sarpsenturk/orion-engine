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
    VulkanDevice::VulkanDevice(spdlog::logger* logger, VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues)
        : RenderDevice(logger)
        , instance_(instance)
        , physical_device_(physical_device)
        , device_(std::move(device))
        , queues_(queues)
    {
        const VmaAllocatorCreateInfo allocator_info{
            .flags = 0,
            .physicalDevice = physical_device,
            .device = this->device(),
            .preferredLargeHeapBlockSize = 0,
            .pHeapSizeLimit = nullptr,
            .pVulkanFunctions = nullptr,
            .instance = instance,
            .vulkanApiVersion = vulkan_api_version,
            .pTypeExternalMemoryHandleTypes = nullptr,
        };
        VmaAllocator allocator = VK_NULL_HANDLE;
        vk_result_check(vmaCreateAllocator(&allocator_info, &allocator));
        allocator_ = vk_unique<UniqueVmaAllocator>(allocator);
    }

    VulkanDevice::~VulkanDevice()
    {
        vkDeviceWaitIdle(device_.get());
    }

    VkQueue VulkanDevice::get_queue(CommandQueueType queue_type) const
    {
        switch (queue_type) {
            case CommandQueueType::Transfer:
                return queues_.transfer.queue;
            case CommandQueueType::Compute:
                return queues_.compute.queue;
            case CommandQueueType::Graphics:
                [[fallthrough]];
            case CommandQueueType::Any:
                return queues_.graphics.queue;
        }
        ORION_ASSERT(!"Invalid queue type");
        return VK_NULL_HANDLE;
    }

    std::uint32_t VulkanDevice::get_queue_family(CommandQueueType queue_type) const
    {
        switch (queue_type) {
            case CommandQueueType::Transfer:
                return queues_.transfer.index;
            case CommandQueueType::Compute:
                return queues_.compute.index;
            case CommandQueueType::Graphics:
                [[fallthrough]];
            case CommandQueueType::Any:
                return queues_.graphics.index;
        }
        ORION_ASSERT(!"Invalid queue type");
        return UINT32_MAX;
    }

    SwapchainHandle VulkanDevice::create_swapchain_api(const Window& window, const SwapchainDesc& desc)
    {
        // Generate handle for swapchain and surface
        auto handle = SwapchainHandle::generate();

        // Create the surface
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        {
            auto unique_surface = create_surface(instance_, window);
            surface = unique_surface.get();
            surfaces_.insert(std::make_pair(handle, std::move(unique_surface)));
        }

        // Chose present mode TODO: Allow user to select this
        const auto present_mode = VK_PRESENT_MODE_FIFO_KHR;

        // Select format
        const auto format = to_vulkan_type(desc.image_format);

        // Create the swapchain
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        {
            // Get the surface capabilities
            const auto surface_capabilities = [physical_device = physical_device_, surface]() {
                VkSurfaceCapabilitiesKHR surface_capabilities;
                vk_result_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities));
                return surface_capabilities;
            }();

            const VkSwapchainCreateInfoKHR info{
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = surface,
                .minImageCount = desc.image_count,
                .imageFormat = format,
                .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
                .imageExtent = to_vulkan_extent(desc.image_size),
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .preTransform = surface_capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = present_mode,
                .clipped = VK_TRUE,
                .oldSwapchain = VK_NULL_HANDLE,
            };
            vk_result_check(vkCreateSwapchainKHR(device(), &info, alloc_callbacks(), &swapchain));
        }

        // Create image views
        std::vector<UniqueVkImageView> image_views;
        {
            // Acquire swapchain images
            auto images = [swapchain, device = device_.get()]() {
                std::uint32_t image_count = 0;
                vk_result_check(vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr));
                std::vector<VkImage> swapchain_images(image_count);
                vk_result_check(vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data()));
                return swapchain_images;
            }();

            image_views.reserve(images.size());
            for (auto image : images) {
                const VkImageViewCreateInfo image_view_info{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .image = image,
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
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
                vk_result_check(vkCreateImageView(device(), &image_view_info, alloc_callbacks(), &image_view));
                image_views.push_back(vk_unique<UniqueVkImageView>(image_view, device()));
            }
        }

        swapchains_.insert(std::make_pair(handle, VulkanSwapchain{
                                                      surface,
                                                      UniqueVkSwapchainKHR{swapchain, SwapchainDeleter{device()}},
                                                      std::move(image_views),
                                                  }));
        return handle;
    }

    RenderPassHandle VulkanDevice::create_render_pass_api(const RenderPassDesc& desc)
    {
        std::vector<VkAttachmentDescription> attachments;
        auto to_vulkan_attachments = [&attachments, attachment_index = 0u](auto input) mutable {
            // Reserve space for new attachment descriptions
            attachments.reserve(attachments.size() + input.size());

            // Return new attachment references
            std::vector<VkAttachmentReference> attachment_refs;
            attachment_refs.reserve(input.size());
            for (const auto& attachment : input) {
                // Create new VkAttachmentDescription
                attachments.push_back({
                    .flags = 0,
                    .format = to_vulkan_type(attachment.format),
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = to_vulkan_type(attachment.load_op),
                    .storeOp = to_vulkan_type(attachment.store_op),
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = to_vulkan_type(attachment.initial_layout),
                    .finalLayout = to_vulkan_type(attachment.final_layout),
                });
                // Create new VkAttachmentReference
                attachment_refs.push_back({
                    .attachment = attachment_index++,
                    .layout = to_vulkan_type(attachment.layout),
                });
            }
            return attachment_refs;
        };

        // Attachment references for subpass
        const auto color_attachments = to_vulkan_attachments(desc.color_attachments);

        const VkSubpassDescription subpass{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = static_cast<std::uint32_t>(color_attachments.size()),
            .pColorAttachments = color_attachments.data(),
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
        };

        const VkRenderPassCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
        };
        VkRenderPass render_pass = VK_NULL_HANDLE;
        vk_result_check(vkCreateRenderPass(device_.get(), &info, alloc_callbacks(), &render_pass));

        auto handle = RenderPassHandle::generate();
        render_passes_.insert(std::make_pair(handle, vk_unique<UniqueVkRenderPass>(render_pass, device())));
        return handle;
    }

    RenderTargetHandle VulkanDevice::create_render_target_api(SwapchainHandle swapchain_handle, const RenderTargetDesc& desc)
    {
        // Find swapchain
        auto& swapchain = find_swapchain(swapchain_handle);

        // Find render pass
        auto render_pass = find_render_pass(desc.render_pass);

        std::vector<UniqueVkFramebuffer> framebuffers;
        framebuffers.reserve(swapchain.image_views().size());
        for (auto& image_view : swapchain.image_views()) {
            // Create framebuffer
            const std::array attachments{image_view.get()};
            const VkFramebufferCreateInfo framebuffer_info{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = render_pass,
                .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = desc.size.x(),
                .height = desc.size.y(),
                .layers = 1,
            };
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            vk_result_check(vkCreateFramebuffer(device(), &framebuffer_info, alloc_callbacks(), &framebuffer));
            framebuffers.push_back(vk_unique<UniqueVkFramebuffer>(framebuffer, device()));
        }

        auto handle = RenderTargetHandle::generate();
        render_targets_.insert(
            std::make_pair(handle,
                           VulkanSwapchainRenderTarget{
                               device(),
                               &swapchain,
                               std::move(framebuffers),
                               create_vk_semaphore(device()),
                           }));
        return handle;
    }

    ShaderModuleHandle VulkanDevice::create_shader_module_api(const ShaderModuleDesc& desc)
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
        vk_result_check(vkCreateShaderModule(device(), &info, alloc_callbacks(), &shader_module));

        auto handle = ShaderModuleHandle::generate();
        shader_modules_.insert(std::make_pair(handle, vk_unique<UniqueVkShaderModule>(shader_module, device())));
        return handle;
    }

    PipelineHandle VulkanDevice::create_graphics_pipeline_api(const GraphicsPipelineDesc& desc)
    {
        // Create descriptor set layout
        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
        {
            const auto descriptor_bindings = [bindings = desc.descriptor_bindings]() {
                std::vector<VkDescriptorSetLayoutBinding> descriptor_bindings;
                descriptor_bindings.reserve(bindings.size());
                for (std::uint32_t index = 0; const auto& binding : bindings) {
                    descriptor_bindings.push_back({
                        .binding = index++,
                        .descriptorType = to_vulkan_type(binding.type),
                        .descriptorCount = binding.count,
                        .stageFlags = to_vulkan_type(binding.shader_stages),
                        .pImmutableSamplers = nullptr,
                    });
                }
                return descriptor_bindings;
            }();
            const VkDescriptorSetLayoutCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .bindingCount = static_cast<std::uint32_t>(descriptor_bindings.size()),
                .pBindings = descriptor_bindings.data(),
            };
            vk_result_check(vkCreateDescriptorSetLayout(device(), &info, alloc_callbacks(), &descriptor_set_layout));
        }

        // Create pipeline layout
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        {
            const VkPipelineLayoutCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = 1,
                .pSetLayouts = &descriptor_set_layout,
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr,
            };
            vk_result_check(vkCreatePipelineLayout(device(), &info, alloc_callbacks(), &pipeline_layout));
        }

        // Convert to VkPipelineShaderStageCreateInfo
        ORION_EXPECTS(desc.shaders.size() <= UINT32_MAX);
        const auto vk_shader_stages = [shaders = desc.shaders, this]() {
            std::vector<VkPipelineShaderStageCreateInfo> vk_shader_stages;
            vk_shader_stages.reserve(shaders.size());
            for (const auto& shader : shaders) {
                vk_shader_stages.push_back({
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .stage = static_cast<VkShaderStageFlagBits>(to_vulkan_type(shader.stage)),
                    .module = find_shader(shader.module),
                    .pName = shader.entry_point,
                    .pSpecializationInfo = nullptr,
                });
            }
            return vk_shader_stages;
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
        VkRenderPass render_pass = find_render_pass(desc.render_pass);

        // Create VkPipeline
        VkPipeline pipeline = VK_NULL_HANDLE;
        {
            const VkGraphicsPipelineCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stageCount = static_cast<std::uint32_t>(vk_shader_stages.size()),
                .pStages = vk_shader_stages.data(),
                .pVertexInputState = &vk_input_state,
                .pInputAssemblyState = &vk_input_assembly,
                .pTessellationState = nullptr,
                .pViewportState = &vk_viewport,
                .pRasterizationState = &vk_rasterization,
                .pMultisampleState = &vk_multisample,
                .pDepthStencilState = nullptr, // No depth stencil support yet
                .pColorBlendState = &vk_color_blend,
                .pDynamicState = &vk_dynamic_state,
                .layout = pipeline_layout,
                .renderPass = render_pass,
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE,
                .basePipelineIndex = 0,
            };
            vk_result_check(vkCreateGraphicsPipelines(device(), VK_NULL_HANDLE, 1, &info, alloc_callbacks(), &pipeline));
        }

        const auto handle = PipelineHandle::generate();
        pipelines_.insert(std::make_pair(handle, VulkanPipeline{
                                                     vk_unique<UniqueVkDescriptorSetLayout>(descriptor_set_layout, device()),
                                                     vk_unique<UniqueVkPipelineLayout>(pipeline_layout, device()),
                                                     vk_unique<UniqueVkPipeline>(pipeline, device()),
                                                 }));
        return handle;
    }

    GPUBufferHandle VulkanDevice::create_buffer_api(const GPUBufferDesc& desc)
    {
        // Check if  buffer will be used for transfer ops
        const bool transfer_src = desc.usage.has(GPUBufferUsage::TransferSrc);
        const bool transfer_dst = desc.usage.has(GPUBufferUsage::TransferDst);

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

    CommandPoolHandle VulkanDevice::create_command_pool_api(const CommandPoolDesc& desc)
    {
        const VkCommandPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = get_queue_family(desc.queue_type),
        };
        VkCommandPool command_pool = VK_NULL_HANDLE;
        vk_result_check(vkCreateCommandPool(device(), &info, alloc_callbacks(), &command_pool));

        auto handle = CommandPoolHandle::generate();
        command_pools_.insert(std::make_pair(handle, UniqueVkCommandPool{command_pool, CommandPoolDeleter{device()}}));
        return handle;
    }

    void VulkanDevice::recreate_api(SwapchainHandle swapchain_handle, const SwapchainDesc& desc)
    {
        // Find existing swapchain
        auto& swapchain = find_swapchain(swapchain_handle);

        // Wait until graphics/presentation queue is idle
        vkQueueWaitIdle(graphics_queue());

        // Chose present mode TODO: Allow user to select this
        const auto present_mode = VK_PRESENT_MODE_FIFO_KHR;

        // Get surface
        auto surface = swapchain.surface();

        const auto format = to_vulkan_type(desc.image_format);

        // Recreate swapchain
        VkSwapchainKHR new_swapchain = VK_NULL_HANDLE;
        {
            // Get the surface capabilities
            const auto surface_capabilities = [physical_device = physical_device_, surface]() {
                VkSurfaceCapabilitiesKHR surface_capabilities;
                vk_result_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities));
                return surface_capabilities;
            }();

            const VkSwapchainCreateInfoKHR info{
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = surface,
                .minImageCount = desc.image_count,
                .imageFormat = format,
                .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
                .imageExtent = to_vulkan_extent(desc.image_size),
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .preTransform = surface_capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = present_mode,
                .clipped = VK_TRUE,
                .oldSwapchain = VK_NULL_HANDLE,
            };
            vk_result_check(vkCreateSwapchainKHR(device(), &info, alloc_callbacks(), &new_swapchain));
        }

        // Create image views
        std::vector<UniqueVkImageView> image_views;
        {
            // Acquire swapchain images
            auto images = [new_swapchain, device = device_.get()]() {
                std::uint32_t image_count = 0;
                vk_result_check(vkGetSwapchainImagesKHR(device, new_swapchain, &image_count, nullptr));
                std::vector<VkImage> swapchain_images(image_count);
                vk_result_check(vkGetSwapchainImagesKHR(device, new_swapchain, &image_count, swapchain_images.data()));
                return swapchain_images;
            }();

            image_views.reserve(images.size());
            for (auto image : images) {
                const VkImageViewCreateInfo image_view_info{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .image = image,
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
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
                vk_result_check(vkCreateImageView(device(), &image_view_info, alloc_callbacks(), &image_view));
                image_views.push_back(vk_unique<UniqueVkImageView>(image_view, device()));
            }
        }

        swapchains_.insert_or_assign(swapchain_handle, VulkanSwapchain{
                                                           swapchain.surface(),
                                                           UniqueVkSwapchainKHR{new_swapchain, SwapchainDeleter{device()}},
                                                           std::move(image_views),
                                                       });
    }

    void VulkanDevice::recreate_api(RenderTargetHandle render_target, SwapchainHandle swapchain, const RenderTargetDesc& desc)
    {
        // Find swapchain
        auto& vk_swapchain = find_swapchain(swapchain);

        // Find render pass
        auto render_pass = find_render_pass(desc.render_pass);

        std::vector<UniqueVkFramebuffer> framebuffers;
        framebuffers.reserve(vk_swapchain.image_views().size());
        for (auto& image_view : vk_swapchain.image_views()) {
            // Create framebuffer
            const std::array attachments{image_view.get()};
            const VkFramebufferCreateInfo framebuffer_info{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = render_pass,
                .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = desc.size.x(),
                .height = desc.size.y(),
                .layers = 1,
            };
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            vk_result_check(vkCreateFramebuffer(device(), &framebuffer_info, alloc_callbacks(), &framebuffer));
            framebuffers.push_back(vk_unique<UniqueVkFramebuffer>(framebuffer, device()));
        }

        render_targets_.insert_or_assign(render_target,
                                         VulkanSwapchainRenderTarget{
                                             device(),
                                             &vk_swapchain,
                                             std::move(framebuffers),
                                             create_vk_semaphore(device()),
                                         });
    }

    CommandBufferHandle VulkanDevice::create_command_buffer_api(const CommandBufferDesc& desc)
    {
        VkCommandPool command_pool = find_command_pool(desc.command_pool);

        const VkCommandBufferAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        vk_result_check(vkAllocateCommandBuffers(device(), &info, &command_buffer));

        const auto handle = CommandBufferHandle::generate();
        command_buffers_.insert(std::make_pair(handle, vk_unique<UniqueVkCommandBuffer>(command_buffer, device(), command_pool)));
        return handle;
    }

    DescriptorPoolHandle VulkanDevice::create_descriptor_pool_api(const DescriptorPoolDesc& desc)
    {
        // Create descriptor pool
        VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
        {
            const auto pool_sizes = [sizes = desc.pool_sizes]() {
                std::vector<VkDescriptorPoolSize> pool_sizes;
                pool_sizes.reserve(sizes.size());
                for (const auto& size : sizes) {
                    pool_sizes.push_back({
                        .type = to_vulkan_type(size.type),
                        .descriptorCount = size.count,
                    });
                }
                return pool_sizes;
            }();
            const VkDescriptorPoolCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .maxSets = desc.max_sets,
                .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
                .pPoolSizes = pool_sizes.data(),
            };
            vk_result_check(vkCreateDescriptorPool(device(), &info, alloc_callbacks(), &descriptor_pool));
        }

        auto handle = DescriptorPoolHandle::generate();
        descriptor_pools_.insert(std::make_pair(handle, vk_unique<UniqueVkDescriptorPool>(descriptor_pool, device())));
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

    VkRenderPass VulkanDevice::find_render_pass(RenderPassHandle render_pass_handle)
    {
        return render_passes_.at(render_pass_handle).get();
    }

    VulkanRenderTarget& VulkanDevice::find_render_target(RenderTargetHandle render_target_handle)
    {
        return render_targets_.at(render_target_handle);
    }

    VkPipeline VulkanDevice::find_pipeline(PipelineHandle pipeline_handle) const
    {
        return pipelines_.at(pipeline_handle).pipeline();
    }

    VkCommandPool VulkanDevice::find_command_pool(CommandPoolHandle command_pool_handle) const
    {
        return command_pools_.at(command_pool_handle).get();
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
        surfaces_.erase(swapchain_handle);
    }

    void VulkanDevice::destroy_api(RenderPassHandle render_pass_handle)
    {
        render_passes_.erase(render_pass_handle);
    }

    void VulkanDevice::destroy_api(RenderTargetHandle render_target_handle)
    {
        render_targets_.erase(render_target_handle);
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

    void VulkanDevice::destroy_api(CommandPoolHandle command_pool_handle)
    {
        command_pools_.erase(command_pool_handle);
    }

    void VulkanDevice::destroy_api(CommandBufferHandle command_buffer_handle)
    {
        command_buffers_.erase(command_buffer_handle);
    }

    void VulkanDevice::destroy_api(SubmissionHandle submission_handle)
    {
        submissions_.erase(submission_handle);
    }

    void VulkanDevice::destroy_api(DescriptorPoolHandle descriptor_pool_handle)
    {
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

    SubmissionHandle VulkanDevice::submit_api(const SubmitDesc& desc)
    {
        const auto* command_buffer = desc.command_buffer;

        // Find and reset command buffer
        VkCommandBuffer vk_command_buffer = find_command_buffer(command_buffer->handle());
        vkResetCommandBuffer(vk_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        // Begin command buffer recording
        const VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };
        vk_result_check(vkBeginCommandBuffer(vk_command_buffer, &begin_info));

        // Find existing or create new submission
        SubmissionHandle submission_handle = desc.existing;
        auto& submission = [this, &submission_handle]() -> VulkanSubmission& {
            if (submission_handle.is_valid()) {
                return find_submission(submission_handle);
            }
            submission_handle = SubmissionHandle::generate();
            auto [iter, _] = submissions_.insert(std::make_pair(submission_handle,
                                                                VulkanSubmission{
                                                                    .fence = create_vk_fence(device_.get()),
                                                                    .semaphore = create_vk_semaphore(device_.get()),
                                                                }));
            return iter->second;
        }();

        // Compile orion commands to vulkan commands
        compile_commands(vk_command_buffer, command_buffer->commands(), submission);

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
        vk_result_check(vkQueueSubmit(get_queue(desc.queue_type), 1, &submit_info, signal_fence));
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
        const auto image_index = swapchain.image_index();

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
        vk_result_check(vkQueuePresentKHR(present_queue, &present_info), VK_SUCCESS, VK_SUBOPTIMAL_KHR);
    }

    void VulkanDevice::compile_commands(VkCommandBuffer command_buffer, const std::vector<CommandPacket>& commands, VulkanSubmission& submission)
    {
        submission.wait_semaphores.clear();
        for (auto& command : commands) {
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
                case CommandType::DrawIndexed:
                    cmd_draw_indexed(command_buffer, command.data);
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
                .srcOffset = cmd_buffer_copy->src_offset,
                .dstOffset = cmd_buffer_copy->dst_offset,
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

        // Find render target
        auto& render_target = find_render_target(cmd_begin_frame->render_target);

        // Find framebuffer
        auto find_framebuffer = [](const auto& render_target) {
            return render_target.framebuffer();
        };
        auto framebuffer = std::visit(find_framebuffer, render_target);

        // Begin render pass
        const VkRenderPassBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = find_render_pass(cmd_begin_frame->render_pass),
            .framebuffer = framebuffer,
            .renderArea = {.offset = {}, .extent = to_vulkan_extent(cmd_begin_frame->render_area)},
            .clearValueCount = static_cast<std::uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data(),
        };
        vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

        // Find and return render target semaphore if exists
        auto find_semaphore = [](const auto& render_target) {
            return render_target.semaphore();
        };
        return std::visit(find_semaphore, render_target);
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

    void VulkanDevice::cmd_draw_indexed(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* const cmd_draw_indexed = static_cast<const CmdDrawIndexed*>(data);

        // Bind graphics pipeline
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, find_pipeline(cmd_draw_indexed->graphics_pipeline));

        // Bind vertex buffer
        VkBuffer vertex_buffer = find_buffer(cmd_draw_indexed->vertex_buffer).buffer();
        const std::array offsets{VkDeviceSize{0}};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offsets.data());

        // Bind index buffer
        VkBuffer index_buffer = find_buffer(cmd_draw_indexed->index_buffer).buffer();
        vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

        // Set viewport and scissor
        const VkViewport viewport = to_vulkan_type(cmd_draw_indexed->viewport);
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        const VkRect2D scissor{.offset = {0, 0}, .extent = {static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height)}};
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        // Issue draw command
        vkCmdDrawIndexed(command_buffer, cmd_draw_indexed->index_count, 1, 0, 0, 0);
    }
} // namespace orion::vulkan
