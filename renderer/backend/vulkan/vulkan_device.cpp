#include "vulkan_device.h"

#include "vulkan_conversion.h"
#include "vulkan_platform.h"

#include "orion-utils/assertion.h"
#include "orion-utils/static_vector.h"

#include <numeric>
#include <ranges>
#include <utility>

#include <spdlog/spdlog.h>

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
        vma_allocator_ = unique(allocator);
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

    std::vector<std::uint32_t> VulkanDevice::get_unique_queue_families(const std::vector<CommandQueueType>& queue_types) const
    {
        std::vector<std::uint32_t> queue_families(queue_types.size());
        std::ranges::transform(queue_types, queue_families.begin(), [this](auto queue_type) {
            return get_queue_family(queue_type);
        });
        queue_families.erase(std::unique(queue_families.begin(), queue_families.end()), queue_families.end());
        return queue_families;
    }

    VkDescriptorSetLayout VulkanDevice::make_descriptor_set_layout(const DescriptorSetLayout& layout)
    {
        const auto hash = layout.hash();
        if (auto iter = descriptor_set_layouts_.find(hash); iter != descriptor_set_layouts_.end()) {
            SPDLOG_LOGGER_TRACE(logger(), "Found cached descriptor set layout");
            return iter->second.resource.get();
        }
        SPDLOG_LOGGER_TRACE(logger(), "Could not find cached descriptor set layout. Creating.");
        VkDescriptorSetLayout vk_layout = create_descriptor_set_layout(layout.bindings());
        descriptor_set_layouts_.add(hash, unique(vk_layout, device()));
        return vk_layout;
    }

    VkDescriptorSetLayout VulkanDevice::create_descriptor_set_layout(std::span<const DescriptorBinding> bindings) const
    {
        std::vector<VkDescriptorSetLayoutBinding> descriptor_bindings(bindings.size());
        std::ranges::transform(bindings, descriptor_bindings.begin(), [index = 0u](const auto& binding) mutable {
            return VkDescriptorSetLayoutBinding{
                .binding = index++,
                .descriptorType = to_vulkan_type(binding.type),
                .descriptorCount = binding.count,
                .stageFlags = to_vulkan_type(binding.shader_stages),
                .pImmutableSamplers = nullptr,
            };
        });

        // Create descriptor set layout
        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
        {
            const VkDescriptorSetLayoutCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .bindingCount = static_cast<std::uint32_t>(descriptor_bindings.size()),
                .pBindings = descriptor_bindings.data(),
            };
            vk_result_check(vkCreateDescriptorSetLayout(device(), &info, alloc_callbacks(), &descriptor_set_layout));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkDescriptorSetLayout {}", fmt::ptr(descriptor_set_layout));
        }
        return descriptor_set_layout;
    }

    VkRenderPass VulkanDevice::create_vkrender_pass(const AttachmentList& attachment_list) const
    {
        // Add all attachment counts
        const auto attachment_count = static_cast<std::uint32_t>(attachment_list.attachment_count());

        // Convert our attachment descriptions to VkAttachmentDescription's
        auto to_attachment = [](const auto& attachment) {
            return VkAttachmentDescription{
                .flags = 0,
                .format = to_vulkan_type(attachment.format),
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = to_vulkan_type(attachment.load_op),
                .storeOp = to_vulkan_type(attachment.store_op),
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = to_vulkan_type(attachment.initial_layout),
                .finalLayout = to_vulkan_type(attachment.final_layout),
            };
        };

        // Insert attachments
        std::vector<VkAttachmentDescription> attachments(attachment_count);
        auto iter = attachments.begin();
        iter = std::ranges::transform(attachment_list.color_attachments, iter, to_attachment).out;

        // Get attachment index offsets
        const auto color_attachment_offset = 0;

        // Convert attachment
        auto to_attachment_ref = [](std::uint32_t attachment_offset) {
            return [index = attachment_offset](const auto& attachment) mutable {
                return VkAttachmentReference{
                    .attachment = index++,
                    .layout = to_vulkan_type(attachment.layout),
                };
            };
        };

        // Create attachment references
        std::vector<VkAttachmentReference> color_attachments(attachment_list.color_attachments.size());
        std::ranges::transform(attachment_list.color_attachments, color_attachments.begin(), to_attachment_ref(color_attachment_offset));

        // Create render pass
        VkRenderPass render_pass = VK_NULL_HANDLE;
        {
            const auto subpass = VkSubpassDescription{
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
            const auto info = VkRenderPassCreateInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .attachmentCount = attachment_count,
                .pAttachments = attachments.data(),
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 0,
                .pDependencies = nullptr,
            };
            vk_result_check(vkCreateRenderPass(device(), &info, alloc_callbacks(), &render_pass));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkRenderPass {}", fmt::ptr(render_pass));
        }
        return render_pass;
    }

    SurfaceHandle VulkanDevice::create_surface_api(const Window& window)
    {
        VkSurfaceKHR surface = create_platform_surface(instance_, window);
        const auto handle = SurfaceHandle::generate();
        surfaces_.add(handle, unique(surface, instance_));
        return handle;
    }

    SwapchainHandle VulkanDevice::create_swapchain_api(const SwapchainDesc& desc)
    {
        // Generate handle for swapchain and surface
        auto handle = SwapchainHandle::generate();

        // Find surface
        VkSurfaceKHR surface = surfaces_.handle_at(desc.surface);

        // Chose present mode TODO: Allow user to select this
        const auto present_mode = VK_PRESENT_MODE_FIFO_KHR;

        // Select format
        const auto format = to_vulkan_type(desc.image_format);

        // Create the swapchain
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        {
            // Get the surface capabilities
            const auto surface_capabilities = [physical_device = physical_device_, &surface]() {
                VkSurfaceCapabilitiesKHR surface_capabilities;
                vk_result_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities));
                return surface_capabilities;
            }();

            // Find set of queue families to be used
            auto queue_types = std::vector{CommandQueueType::Graphics};
            const auto has_transfer = !!(desc.image_usage & ImageUsageFlags::Transfer);
            if (has_transfer) {
                queue_types.push_back(CommandQueueType::Transfer);
            }
            const auto queue_families = get_unique_queue_families(queue_types);
            const auto sharing_mode = queue_families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

            const auto info = VkSwapchainCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = surface,
                .minImageCount = desc.image_count,
                .imageFormat = format,
                .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
                .imageExtent = to_vulkan_extent(desc.image_size),
                .imageArrayLayers = 1,
                .imageUsage = to_vulkan_type(desc.image_usage),
                .imageSharingMode = sharing_mode,
                .queueFamilyIndexCount = static_cast<std::uint32_t>(queue_families.size()),
                .pQueueFamilyIndices = queue_families.data(),
                .preTransform = surface_capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = present_mode,
                .clipped = VK_TRUE,
                .oldSwapchain = VK_NULL_HANDLE,
            };
            vk_result_check(vkCreateSwapchainKHR(device(), &info, alloc_callbacks(), &swapchain));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkSwapchain {}", fmt::ptr(swapchain));
        }

        std::vector<ImageHandle> image_handles;
        // Get swapchain images
        {
            std::uint32_t image_count = 0;
            vk_result_check(vkGetSwapchainImagesKHR(device(), swapchain, &image_count, nullptr));
            std::vector<VkImage> images(image_count);
            vk_result_check(vkGetSwapchainImagesKHR(device(), swapchain, &image_count, images.data()));
            for (VkImage image : images) {
                const auto image_handle = ImageHandle::generate();
                images_.add(image_handle, unique(image, VK_NULL_HANDLE, VK_NULL_HANDLE, false));
                image_handles.push_back(image_handle);
            }
        }

        swapchains_.add(handle, unique(swapchain, device()), {image_handles});
        return handle;
    }

    RenderPassHandle VulkanDevice::create_render_pass_api(const RenderPassDesc& desc)
    {
        VkRenderPass render_pass = create_vkrender_pass(desc.attachments);

        auto handle = RenderPassHandle::generate();
        render_passes_.add(handle, unique(render_pass, device()));
        return handle;
    }

    FramebufferHandle VulkanDevice::create_framebuffer_api(const FramebufferDesc& desc)
    {
        // Find image views
        std::vector<VkImageView> image_views(desc.image_views.size());
        std::ranges::transform(desc.image_views, image_views.begin(), [this](auto handle) {
            return image_views_.handle_at(handle);
        });

        // Create temporary compatible render pass
        VkRenderPass render_pass = create_vkrender_pass(desc.attachment_list);

        // Create framebuffer
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        {
            const auto info = VkFramebufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = render_pass,
                .attachmentCount = static_cast<std::uint32_t>(image_views.size()),
                .pAttachments = image_views.data(),
                .width = desc.size.x(),
                .height = desc.size.y(),
                .layers = 1,
            };
            vk_result_check(vkCreateFramebuffer(device(), &info, alloc_callbacks(), &framebuffer));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkFramebuffer {}", fmt::ptr(framebuffer));
        }

        // Destroy temporary render pass
        vkDestroyRenderPass(device(), render_pass, alloc_callbacks());

        const auto handle = FramebufferHandle::generate();
        framebuffers_.add(handle, unique(framebuffer, device()));
        return handle;
    }

    ShaderModuleHandle VulkanDevice::create_shader_module_api(const ShaderModuleDesc& desc)
    {
        // Convert to std::byte data to uint32_t
        std::vector<std::uint32_t> spirv(desc.byte_code.size_bytes() / sizeof(std::uint32_t));
        std::memcpy(spirv.data(), desc.byte_code.data(), desc.byte_code.size_bytes());

        // Create shader module
        VkShaderModule shader_module = VK_NULL_HANDLE;
        {
            const VkShaderModuleCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .codeSize = desc.byte_code.size_bytes(),
                .pCode = spirv.data(),
            };
            vk_result_check(vkCreateShaderModule(device(), &info, alloc_callbacks(), &shader_module));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkShaderModule {}", fmt::ptr(shader_module));
        }

        auto handle = ShaderModuleHandle::generate();
        shader_modules_.add(handle, unique(shader_module, device()));
        return handle;
    }

    PipelineHandle VulkanDevice::create_graphics_pipeline_api(const GraphicsPipelineDesc& desc)
    {
        // Generate handle for pipeline layout and pipeline
        // TODO: Instead cache pipeline layouts
        const auto handle = PipelineHandle::generate();

        // Create pipeline layout
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        {
            // Create or get descriptor set layouts
            std::vector<VkDescriptorSetLayout> descriptor_set_layouts(desc.descriptor_layouts.size());
            std::ranges::transform(desc.descriptor_layouts, descriptor_set_layouts.begin(), [this](const auto& layout) {
                return make_descriptor_set_layout(layout);
            });

            // Get push constant ranges
            std::vector<VkPushConstantRange> push_constants(desc.push_constants.size());
            std::ranges::transform(desc.push_constants, push_constants.begin(), [offset = 0u](const auto& push_constant) mutable {
                auto push_constant_range = VkPushConstantRange{
                    .stageFlags = to_vulkan_type(push_constant.shader_stages),
                    .offset = offset,
                    .size = static_cast<std::uint32_t>(push_constant.size),
                };
                offset += static_cast<std::uint32_t>(push_constant.size);
                return push_constant_range;
            });

            const auto info = VkPipelineLayoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = static_cast<std::uint32_t>(descriptor_set_layouts.size()),
                .pSetLayouts = descriptor_set_layouts.data(),
                .pushConstantRangeCount = static_cast<std::uint32_t>(push_constants.size()),
                .pPushConstantRanges = push_constants.data(),
            };
            vk_result_check(vkCreatePipelineLayout(device(), &info, alloc_callbacks(), &pipeline_layout));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkPipelineLayout {}", fmt::ptr(pipeline_layout));
        }
        pipeline_layouts_.add(handle, unique(pipeline_layout, device()));

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
                    .module = shader_modules_.handle_at(shader.module),
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

        // Convert color blend attachments to VkPipelineColorBlendAttachmentState
        std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachments(desc.color_blend.attachments.size());
        std::ranges::transform(desc.color_blend.attachments, vk_blend_attachments.begin(), [](const auto& attachment) {
            return VkPipelineColorBlendAttachmentState{
                .blendEnable = attachment.enable_blend,
                .srcColorBlendFactor = to_vulkan_type(attachment.src_blend),
                .dstColorBlendFactor = to_vulkan_type(attachment.dst_blend),
                .colorBlendOp = to_vulkan_type(attachment.blend_op),
                .srcAlphaBlendFactor = to_vulkan_type(attachment.src_blend_alpha),
                .dstAlphaBlendFactor = to_vulkan_type(attachment.dst_blend_alpha),
                .alphaBlendOp = to_vulkan_type(attachment.blend_op_alpha),
                .colorWriteMask = to_vulkan_type(attachment.color_component_flags),
            };
        });

        const auto& blend_constants = desc.color_blend.blend_constants;
        const auto vk_color_blend = VkPipelineColorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = desc.color_blend.enable_logic_op,
            .logicOp = to_vulkan_type(desc.color_blend.logic_op),
            .attachmentCount = static_cast<std::uint32_t>(vk_blend_attachments.size()),
            .pAttachments = vk_blend_attachments.data(),
            .blendConstants = {blend_constants[0], blend_constants[1], blend_constants[2], blend_constants[3]},
        };

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

        // Create temporary compatible render pass
        VkRenderPass render_pass = create_vkrender_pass(desc.attachment_list);

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
            SPDLOG_LOGGER_TRACE(logger(), "Created VkPipeline {}", fmt::ptr(pipeline));
        }

        // Destroy temporary render pass
        vkDestroyRenderPass(device(), render_pass, alloc_callbacks());

        pipelines_.add(handle, unique(pipeline, device()));
        return handle;
    }

    GPUBufferHandle VulkanDevice::create_buffer_api(const GPUBufferDesc& desc)
    {
        // Find set of queue families to be used
        auto queue_types = std::vector{CommandQueueType::Graphics};
        const auto has_transfer = !!(desc.usage & GPUBufferUsageFlags::Transfer);
        if (has_transfer) {
            queue_types.push_back(CommandQueueType::Transfer);
        }
        const auto queue_families = get_unique_queue_families(queue_types);
        const auto sharing_mode = queue_families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

        // Create buffer and allocation
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        {
            const auto buffer_info = VkBufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .size = desc.size,
                .usage = to_vulkan_type(desc.usage),
                .sharingMode = sharing_mode,
                .queueFamilyIndexCount = static_cast<std::uint32_t>(queue_families.size()),
                .pQueueFamilyIndices = queue_families.data(),
            };
            const auto allocation_info = VmaAllocationCreateInfo{
                .flags = desc.host_visible ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : VmaAllocationCreateFlags{},
                .usage = VMA_MEMORY_USAGE_AUTO,
                .requiredFlags = 0,
                .preferredFlags = 0,
                .pool = VK_NULL_HANDLE,
                .pUserData = nullptr,
                .priority = 0.f,
            };
            vk_result_check(vmaCreateBuffer(vma_allocator(), &buffer_info, &allocation_info, &buffer, &allocation, nullptr));
        }

        const auto handle = GPUBufferHandle::generate();
        buffers_.add(handle, unique(buffer, vma_allocator(), allocation));
        // Allocations are stored alongside resources in a
        // type agnostic way.
        allocations_.insert(std::make_pair(handle.value(), allocation));
        return handle;
    }

    CommandPoolHandle VulkanDevice::create_command_pool_api(const CommandPoolDesc& desc)
    {
        VkCommandPool command_pool = VK_NULL_HANDLE;
        {
            const auto info = VkCommandPoolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = get_queue_family(desc.queue_type),
            };
            vk_result_check(vkCreateCommandPool(device(), &info, alloc_callbacks(), &command_pool));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkCommandPool {}", fmt::ptr(command_pool));
        }

        auto handle = CommandPoolHandle::generate();
        command_pools_.add(handle, unique(command_pool, device()));
        return handle;
    }

    CommandBufferHandle VulkanDevice::create_command_buffer_api(const CommandBufferDesc& desc)
    {
        VkCommandPool command_pool = command_pools_.handle_at(desc.command_pool);

        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        {
            const auto info = VkCommandBufferAllocateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            vk_result_check(vkAllocateCommandBuffers(device(), &info, &command_buffer));
            SPDLOG_LOGGER_TRACE(logger(), "Allocated VkCommandBuffer {}", fmt::ptr(command_buffer));
        }

        const auto handle = CommandBufferHandle::generate();
        command_buffers_.add(handle, unique(command_buffer, device(), command_pool));
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
            const auto info = VkDescriptorPoolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                .maxSets = desc.max_sets,
                .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
                .pPoolSizes = pool_sizes.data(),
            };
            vk_result_check(vkCreateDescriptorPool(device(), &info, alloc_callbacks(), &descriptor_pool));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkDescriptorPool {}", fmt::ptr(descriptor_pool));
        }

        auto handle = DescriptorPoolHandle::generate();
        descriptor_pools_.add(handle, unique(descriptor_pool, device()));
        return handle;
    }

    DescriptorSetHandle VulkanDevice::create_descriptor_set_api(const DescriptorSetDesc& desc)
    {
        VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
        VkDescriptorPool pool = descriptor_pools_.handle_at(desc.descriptor_pool);
        // Create descriptor set
        {
            VkDescriptorSetLayout layout = make_descriptor_set_layout(*desc.layout);
            const auto info = VkDescriptorSetAllocateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &layout,
            };
            vk_result_check(vkAllocateDescriptorSets(device(), &info, &descriptor_set));
            SPDLOG_LOGGER_TRACE(logger(), "Allocated VkDescriptorSet {}", fmt::ptr(descriptor_set));
        }
        auto handle = DescriptorSetHandle::generate();
        descriptor_sets_.add(handle, unique(descriptor_set, device(), pool));
        return handle;
    }

    SemaphoreHandle VulkanDevice::create_semaphore_api()
    {
        VkSemaphore semaphore = VK_NULL_HANDLE;
        {
            const auto info = VkSemaphoreCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
            };
            vk_result_check(vkCreateSemaphore(device(), &info, alloc_callbacks(), &semaphore));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkSemaphore {}", fmt::ptr(semaphore));
        }

        auto handle = SemaphoreHandle::generate();
        semaphores_.add(handle, unique(semaphore, device()));
        return handle;
    }

    FenceHandle VulkanDevice::create_fence_api(bool create_signaled)
    {
        VkFence fence = VK_NULL_HANDLE;
        {
            const auto info = VkFenceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = create_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags{}};
            vk_result_check(vkCreateFence(device(), &info, alloc_callbacks(), &fence));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkFence {}", fmt::ptr(fence));
        }

        auto handle = FenceHandle::generate();
        fences_.add(handle, unique(fence, device()));
        return handle;
    }

    ImageHandle VulkanDevice::create_image_api(const ImageDesc& desc)
    {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        {
            auto queue_types = std::vector{CommandQueueType::Graphics};
            const auto has_transfer = !!(desc.usage & ImageUsageFlags::Transfer);
            if (has_transfer) {
                queue_types.push_back(CommandQueueType::Transfer);
            }
            const auto queue_families = get_unique_queue_families(queue_types);
            const auto sharing_mode = queue_families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

            const auto image_info = VkImageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .imageType = to_vulkan_type(desc.type),
                .format = to_vulkan_type(desc.format),
                .extent = to_vulkan_extent(desc.size),
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = to_vulkan_type(desc.tiling),
                .usage = to_vulkan_type(desc.usage),
                .sharingMode = sharing_mode,
                .queueFamilyIndexCount = static_cast<std::uint32_t>(queue_families.size()),
                .pQueueFamilyIndices = queue_families.data(),
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            };
            const auto allocation_info = VmaAllocationCreateInfo{
                .usage = VMA_MEMORY_USAGE_AUTO,
            };

            vk_result_check(vmaCreateImage(vma_allocator(), &image_info, &allocation_info, &image, &allocation, nullptr));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkImage {}", fmt::ptr(image));
        }
        const auto handle = ImageHandle::generate();
        images_.add(handle, unique(image, vma_allocator(), allocation, true));
        return handle;
    }

    ImageViewHandle VulkanDevice::create_image_view_api(const ImageViewDesc& desc)
    {
        VkImageView image_view = VK_NULL_HANDLE;
        {
            const auto info = VkImageViewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = images_.handle_at(desc.image),
                .viewType = to_vulkan_type(desc.type),
                .format = to_vulkan_type(desc.format),
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // TODO: Allow this to be customized
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            vk_result_check(vkCreateImageView(device(), &info, alloc_callbacks(), &image_view));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkImageView {}", fmt::ptr(image_view));
        }
        const auto handle = ImageViewHandle::generate();
        image_views_.add(handle, unique(image_view, device()));
        return handle;
    }

    SamplerHandle VulkanDevice::create_sampler_api(const SamplerDesc& desc)
    {
        VkSampler sampler = VK_NULL_HANDLE;
        {
            const auto filter = to_vulkan_type(desc.filter);
            const auto info = VkSamplerCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .magFilter = filter,
                .minFilter = filter,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, // TODO: Make this customizable
                .addressModeU = to_vulkan_type(desc.address_mode_u),
                .addressModeV = to_vulkan_type(desc.address_mode_v),
                .addressModeW = to_vulkan_type(desc.address_mode_w),
                .mipLodBias = desc.mip_load_bias,
                .anisotropyEnable = VK_FALSE, // TODO: Make this customizable
                .maxAnisotropy = desc.max_anisotropy,
                .compareEnable = VK_TRUE,
                .compareOp = to_vulkan_type(desc.compare_func),
                .minLod = desc.min_lod,
                .maxLod = desc.max_lod,
                .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, // TODO: Make this customizable
                .unnormalizedCoordinates = VK_FALSE,
            };
            vk_result_check(vkCreateSampler(device(), &info, alloc_callbacks(), &sampler));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkSampler {}", fmt::ptr(sampler));
        }
        const auto handle = SamplerHandle::generate();
        samplers_.add(handle, unique(sampler, device()));
        return handle;
    }

    void VulkanDevice::destroy_api(SurfaceHandle surface_handle)
    {
        surfaces_.remove(surface_handle);
    }

    void VulkanDevice::destroy_api(SwapchainHandle swapchain_handle)
    {
        swapchains_.remove(swapchain_handle);
    }

    void VulkanDevice::destroy_api(RenderPassHandle render_pass_handle)
    {
        render_passes_.remove(render_pass_handle);
    }

    void VulkanDevice::destroy_api(FramebufferHandle framebuffer_handle)
    {
        framebuffers_.remove(framebuffer_handle);
    }

    void VulkanDevice::destroy_api(ShaderModuleHandle shader_module_handle)
    {
        shader_modules_.remove(shader_module_handle);
    }

    void VulkanDevice::destroy_api(PipelineHandle graphics_pipeline_handle)
    {
        pipelines_.remove(graphics_pipeline_handle);
    }

    void VulkanDevice::destroy_api(GPUBufferHandle buffer_handle)
    {
        buffers_.remove(buffer_handle);
        allocations_.erase(buffer_handle.value());
    }

    void VulkanDevice::destroy_api(CommandPoolHandle command_pool_handle)
    {
        command_pools_.remove(command_pool_handle);
    }

    void VulkanDevice::destroy_api(CommandBufferHandle command_buffer_handle)
    {
        command_buffers_.remove(command_buffer_handle);
    }

    void VulkanDevice::destroy_api(DescriptorPoolHandle descriptor_pool_handle)
    {
        descriptor_pools_.remove(descriptor_pool_handle);
    }

    void VulkanDevice::destroy_api(DescriptorSetHandle descriptor_set_handle)
    {
        descriptor_sets_.remove(descriptor_set_handle);
    }

    void VulkanDevice::destroy_api(SemaphoreHandle semaphore_handle)
    {
        semaphores_.remove(semaphore_handle);
    }

    void VulkanDevice::destroy_api(FenceHandle fence_handle)
    {
        fences_.remove(fence_handle);
    }

    void VulkanDevice::destroy_api(ImageHandle image_handle)
    {
        images_.remove(image_handle);
    }

    void VulkanDevice::destroy_api(ImageViewHandle image_view_handle)
    {
        image_views_.remove(image_view_handle);
    }

    void VulkanDevice::destroy_api(SamplerHandle sampler_handle)
    {
        samplers_.remove(sampler_handle);
    }

    void* VulkanDevice::map_api(GPUBufferHandle buffer_handle)
    {
        void* ptr = nullptr;
        VmaAllocation allocation = allocations_.at(buffer_handle.value());
        vk_result_check(vmaMapMemory(vma_allocator_.get(), allocation, &ptr));
        SPDLOG_LOGGER_TRACE(logger(), "Mapped VmaAllocation {} at memory address {}", fmt::ptr(allocation), fmt::ptr(ptr));
        return ptr;
    }

    void VulkanDevice::unmap_api(GPUBufferHandle buffer_handle)
    {
        vmaUnmapMemory(vma_allocator_.get(), allocations_.at(buffer_handle.value()));
    }

    void VulkanDevice::reset_command_pool_api(CommandPoolHandle command_pool)
    {
        vk_result_check(vkResetCommandPool(device(), command_pools_.handle_at(command_pool), 0));
    }

    void VulkanDevice::begin_command_buffer_api(CommandBufferHandle command_buffer, const CommandBufferBeginDesc& desc)
    {
        // Find and reset command buffer
        VkCommandBuffer vk_command_buffer = command_buffers_.handle_at(command_buffer);
        vkResetCommandBuffer(vk_command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        // Begin command buffer recording
        const auto begin_info = VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = to_vulkan_type(desc.usage),
            .pInheritanceInfo = nullptr,
        };
        vk_result_check(vkBeginCommandBuffer(vk_command_buffer, &begin_info));
    }

    void VulkanDevice::end_command_buffer_api(CommandBufferHandle command_buffer)
    {
        vk_result_check(vkEndCommandBuffer(command_buffers_.handle_at(command_buffer)));
    }

    void VulkanDevice::reset_command_buffer_api(CommandBufferHandle command_buffer)
    {
        vkResetCommandBuffer(command_buffers_.handle_at(command_buffer), 0);
    }

    void VulkanDevice::compile_commands_api(CommandBufferHandle command_buffer, std::span<const CommandPacket> commands)
    {
        // Find command buffer
        VkCommandBuffer vk_command_buffer = command_buffers_.handle_at(command_buffer);

        // Compile commands
        for (const auto& command : commands) {
            compile_command(vk_command_buffer, command);
        }
    }

    void VulkanDevice::submit_api(const SubmitDesc& desc)
    {
        // Find command buffers
        std::vector<VkCommandBuffer> command_buffers(desc.command_buffers.size());
        std::ranges::transform(desc.command_buffers, command_buffers.begin(), [this](auto handle) {
            return command_buffers_.handle_at(handle);
        });

        auto find_semaphore_fn = [this](auto handle) { return semaphores_.handle_at(handle); };
        // Find wait semaphores
        std::vector<VkSemaphore> wait_semaphores(desc.wait_semaphores.size());
        std::ranges::transform(desc.wait_semaphores, wait_semaphores.begin(), find_semaphore_fn);
        // Find signal semaphores
        std::vector<VkSemaphore> signal_semaphores(desc.signal_semaphores.size());
        std::ranges::transform(desc.signal_semaphores, signal_semaphores.begin(), find_semaphore_fn);

        // Find signal fence
        VkFence signal_fence = fences_.handle_or_null(desc.fence);

        // Set wait stages
        std::vector<VkPipelineStageFlags> wait_stages(desc.wait_stages.size());
        std::ranges::transform(desc.wait_stages, wait_stages.begin(), [](auto stage) {
            return to_vulkan_type(stage);
        });

        // Submit command buffer
        const VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<std::uint32_t>(wait_semaphores.size()),
            .pWaitSemaphores = wait_semaphores.data(),
            .pWaitDstStageMask = wait_stages.data(),
            .commandBufferCount = static_cast<std::uint32_t>(command_buffers.size()),
            .pCommandBuffers = command_buffers.data(),
            .signalSemaphoreCount = static_cast<std::uint32_t>(signal_semaphores.size()),
            .pSignalSemaphores = signal_semaphores.data(),
        };
        vk_result_check(vkQueueSubmit(get_queue(desc.queue_type), 1, &submit_info, signal_fence));
    }

    void VulkanDevice::present_api(const SwapchainPresentDesc& desc)
    {
        // Find presentation queue
        VkQueue present_queue = get_queue(CommandQueueType::Graphics);

        // Find swapchain and resources
        VkSwapchainKHR swapchain = swapchains_.handle_at(desc.swapchain);

        // Wait semaphore
        VkSemaphore semaphore = semaphores_.handle_at(desc.wait_semaphore);

        // Present image
        const VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &desc.image_index,
            .pResults = nullptr,
        };
        vk_result_check(vkQueuePresentKHR(present_queue, &present_info), {VK_SUCCESS, VK_SUBOPTIMAL_KHR});
    }

    void VulkanDevice::wait_for_fence_api(FenceHandle fence)
    {
        VkFence vk_fence = fences_.handle_at(fence);
        vk_result_check(
            vkWaitForFences(device(), 1, &vk_fence, VK_TRUE, UINT64_MAX),
            {VK_SUCCESS, VK_TIMEOUT});
        vk_result_check(vkResetFences(device(), 1, &vk_fence));
    }

    void VulkanDevice::wait_queue_idle_api(CommandQueueType queue_type)
    {
        vk_result_check(vkQueueWaitIdle(get_queue(queue_type)));
    }

    void VulkanDevice::wait_idle_api()
    {
        vk_result_check(vkDeviceWaitIdle(device()));
    }

    void VulkanDevice::update_descriptor_sets_api(std::span<const DescriptorSetUpdate> updates)
    {
        std::vector<VkDescriptorBufferInfo> buffer_updates;
        std::vector<VkDescriptorImageInfo> image_updates;
        std::vector<VkWriteDescriptorSet> descriptor_writes(updates.size());
        std::ranges::transform(updates, descriptor_writes.begin(), [&buffer_updates, &image_updates, this](const auto& update) {
            VkDescriptorImageInfo* image_info = nullptr;
            VkDescriptorBufferInfo* buffer_info = nullptr;
            if (update.buffer_handle.is_valid()) {
                buffer_updates.push_back({
                    .buffer = buffers_.handle_at(update.buffer_handle),
                    .offset = update.buffer_offset,
                    .range = update.buffer_size,
                });
                buffer_info = &(buffer_updates.back());
            } else if (update.image_view.is_valid() || update.sampler.is_valid()) {
                image_updates.push_back({
                    .sampler = samplers_.handle_or_null(update.sampler),
                    .imageView = image_views_.handle_or_null(update.image_view),
                    .imageLayout = to_vulkan_type(update.image_layout),
                });
                image_info = &(image_updates.back());
            }

            return VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_sets_.handle_at(update.descriptor_set),
                .dstBinding = update.binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = to_vulkan_type(update.descriptor_type),
                .pImageInfo = image_info,
                .pBufferInfo = buffer_info,
                .pTexelBufferView = nullptr,
            };
        });
        vkUpdateDescriptorSets(
            device(),
            static_cast<std::uint32_t>(descriptor_writes.size()), descriptor_writes.data(),
            0u, nullptr);
    }

    uint32_t VulkanDevice::acquire_next_image_api(SwapchainHandle swapchain, SemaphoreHandle semaphore, FenceHandle fence)
    {
        // Find swapchain
        VkSwapchainKHR vk_swapchain = swapchains_.handle_at(swapchain);

        // Find semaphore and fence if valid
        VkSemaphore vk_semaphore = semaphore.is_valid() ? semaphores_.handle_at(semaphore) : VK_NULL_HANDLE;
        VkFence vk_fence = fence.is_valid() ? fences_.handle_at(fence) : VK_NULL_HANDLE;

        // Get image index
        std::uint32_t image_index = 0;
        vk_result_check(
            vkAcquireNextImageKHR(device(), vk_swapchain, UINT64_MAX, vk_semaphore, vk_fence, &image_index),
            {VK_SUCCESS, VK_SUBOPTIMAL_KHR});
        return image_index;
    }

    ImageHandle VulkanDevice::get_swapchain_image_api(SwapchainHandle swapchain, std::uint32_t image_index)
    {
        // Find swapchain images
        const auto& images = swapchains_.data_at(swapchain).images;

        // Return image at index
        return images[image_index];
    }

    void VulkanDevice::compile_command(VkCommandBuffer command_buffer, const CommandPacket& command_packet)
    {
        switch (command_packet.type) {
            case CommandType::BufferCopy:
                cmd_buffer_copy(command_buffer, command_packet.data);
                break;
            case CommandType::BeginRenderPass:
                cmd_begin_render_pass(command_buffer, command_packet.data);
                break;
            case CommandType::EndRenderPass:
                cmd_end_render_pass(command_buffer, command_packet.data);
                break;
            case CommandType::Draw:
                cmd_draw(command_buffer, command_packet.data);
                break;
            case CommandType::DrawIndexed:
                cmd_draw_indexed(command_buffer, command_packet.data);
                break;
            case CommandType::BindDescriptorSet:
                cmd_bind_descriptor_set(command_buffer, command_packet.data);
                break;
            case CommandType::PipelineBarrier:
                cmd_pipeline_barrier(command_buffer, command_packet.data);
                break;
            case CommandType::BlitImage:
                cmd_blit_image(command_buffer, command_packet.data);
                break;
            case CommandType::PushConstants:
                cmd_push_constants(command_buffer, command_packet.data);
                break;
            case CommandType::CopyBufferToImage:
                cmd_copy_buffer_to_image(command_buffer, command_packet.data);
                break;
            default:
                ORION_ASSERT(!"Command type not handled!");
        }
    }

    void VulkanDevice::cmd_buffer_copy(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* cmd_data = static_cast<const CmdBufferCopy*>(data);

        // Find related buffers
        VkBuffer src_buffer = buffers_.handle_at(cmd_data->src);
        VkBuffer dst_buffer = buffers_.handle_at(cmd_data->dst);

        // Set copy regions
        const auto regions = std::array{
            VkBufferCopy{
                .srcOffset = cmd_data->src_offset,
                .dstOffset = cmd_data->dst_offset,
                .size = cmd_data->size,
            },
        };

        // Issue command
        vkCmdCopyBuffer(
            command_buffer,
            src_buffer, dst_buffer,
            static_cast<std::uint32_t>(regions.size()), regions.data());
    }

    void VulkanDevice::cmd_begin_render_pass(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* cmd_data = static_cast<const CmdBeginRenderPass*>(data);

        // Find render pass
        VkRenderPass render_pass = render_passes_.handle_at(cmd_data->render_pass);
        // Find frame buffer
        VkFramebuffer framebuffer = framebuffers_.handle_at(cmd_data->framebuffer);

        // Set clear values
        const auto clear_values = std::array{
            VkClearValue{.color = {cmd_data->clear_color[0], cmd_data->clear_color[1], cmd_data->clear_color[2], cmd_data->clear_color[3]}},
        };

        // Issue command
        const auto info = VkRenderPassBeginInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = render_pass,
            .framebuffer = framebuffer,
            .renderArea = {.offset = {}, .extent = to_vulkan_extent(cmd_data->render_area)},
            .clearValueCount = static_cast<std::uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data(),
        };
        vkCmdBeginRenderPass(command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanDevice::cmd_end_render_pass(VkCommandBuffer command_buffer, const void* data)
    {
        (void)data;
        vkCmdEndRenderPass(command_buffer);
    }

    void VulkanDevice::cmd_draw(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* cmd_data = static_cast<const CmdDraw*>(data);

        // Find and bind the pipeline
        VkPipeline pipeline = pipelines_.handle_at(cmd_data->graphics_pipeline);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // Set viewport
        const auto viewport = to_vulkan_viewport(cmd_data->viewport);
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        // Set scissor
        const auto scissor = VkRect2D{
            .offset = {},
            .extent = to_vulkan_extent(cmd_data->viewport.size),
        };
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        // Find and bind vertex buffer
        VkBuffer vertex_buffer = buffers_.handle_at(cmd_data->vertex_buffer);
        const auto offset = VkDeviceSize{};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, &offset);

        // Issue command
        vkCmdDraw(
            command_buffer,
            cmd_data->vertex_count,
            1u,
            cmd_data->first_vertex,
            0u);
    }

    void VulkanDevice::cmd_draw_indexed(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* cmd_data = static_cast<const CmdDrawIndexed*>(data);

        // Find and bind the pipeline
        VkPipeline pipeline = pipelines_.handle_at(cmd_data->graphics_pipeline);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // Set viewport
        const auto viewport = to_vulkan_viewport(cmd_data->viewport);
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        // Set scissor
        const auto scissor = to_vulkan_rect(cmd_data->scissor);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        // Find and bind vertex buffer
        VkBuffer vertex_buffer = buffers_.handle_at(cmd_data->vertex_buffer);
        const auto offset = VkDeviceSize{};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, &offset);

        // Find and bind index buffer
        VkBuffer index_buffer = buffers_.handle_at(cmd_data->index_buffer);
        vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, to_vulkan_type(cmd_data->index_type));

        // Issue command
        vkCmdDrawIndexed(
            command_buffer,
            cmd_data->index_count,
            1u,
            cmd_data->index_offset,
            cmd_data->vertex_offset,
            0u);
    }

    void VulkanDevice::cmd_bind_descriptor_set(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* cmd_data = static_cast<const CmdBindDescriptorSet*>(data);

        // Set bind point TODO: Make this customizable
        const auto bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;

        // Find pipeline layout
        VkPipelineLayout pipeline_layout = pipeline_layouts_.handle_at(cmd_data->pipeline);

        // Find descriptor set
        VkDescriptorSet descriptor_set = descriptor_sets_.handle_at(cmd_data->descriptor_set);

        // Issue command
        vkCmdBindDescriptorSets(
            command_buffer,
            bind_point,
            pipeline_layout,
            cmd_data->binding,
            1u,
            &descriptor_set,
            0,
            nullptr);
    }

    void VulkanDevice::cmd_pipeline_barrier(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* cmd_data = static_cast<const CmdPipelineBarrier*>(data);

        const auto image_memory_barrier = VkImageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = to_vulkan_type(cmd_data->image_barrier.src_access),
            .dstAccessMask = to_vulkan_type(cmd_data->image_barrier.dst_access),
            .oldLayout = to_vulkan_type(cmd_data->image_barrier.old_layout),
            .newLayout = to_vulkan_type(cmd_data->image_barrier.new_layout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = images_.handle_at(cmd_data->image_barrier.image),
            // TODO: Allow this to be customized
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        vkCmdPipelineBarrier(
            command_buffer,
            to_vulkan_type(cmd_data->src_stages),
            to_vulkan_type(cmd_data->dst_stages),
            0,
            0u, nullptr,
            0u, nullptr,
            1u, &image_memory_barrier);
    }

    void VulkanDevice::cmd_blit_image(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* cmd_data = static_cast<const CmdBlitImage*>(data);

        // Find source image
        VkImage src_image = images_.handle_at(cmd_data->src_image);

        // Find destination image
        VkImage dst_image = images_.handle_at(cmd_data->dst_image);

        // Get sizes
        const auto src_size = vector_cast<int>(cmd_data->src_size);
        const auto dst_size = vector_cast<int>(cmd_data->dst_size);

        // Blit info
        // TODO: Allow for customization of sub resources and offsets
        const auto blit = VkImageBlit{
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .srcOffsets = {
                VkOffset3D{0, 0, 0},
                VkOffset3D{src_size.x(), src_size.y(), 1},
            },
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .dstOffsets = {
                VkOffset3D{0, 0, 0},
                VkOffset3D{dst_size.x(), dst_size.y(), 1},
            },
        };

        vkCmdBlitImage(
            command_buffer,
            src_image,
            to_vulkan_type(cmd_data->src_layout),
            dst_image,
            to_vulkan_type(cmd_data->dst_layout),
            1,
            &blit,
            VK_FILTER_NEAREST);
    }

    void VulkanDevice::cmd_push_constants(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* cmd_data = static_cast<const CmdPushConstants*>(data);

        // Find pipeline layout
        VkPipelineLayout pipeline_layout = pipeline_layouts_.handle_at(cmd_data->pipeline);

        // Push the constants
        vkCmdPushConstants(
            command_buffer,
            pipeline_layout,
            to_vulkan_type(cmd_data->shader_stages),
            static_cast<std::uint32_t>(cmd_data->offset),
            static_cast<std::uint32_t>(cmd_data->size),
            cmd_data->data);
    }

    void VulkanDevice::cmd_copy_buffer_to_image(VkCommandBuffer command_buffer, const void* data)
    {
        const auto* cmd_data = static_cast<const CmdCopyBufferToImage*>(data);

        // Find source buffer
        VkBuffer src_buffer = buffers_.handle_at(cmd_data->src_buffer);

        // Find destination image
        VkImage dst_image = images_.handle_at(cmd_data->dst_image);

        const auto copy = VkBufferImageCopy{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .imageOffset = {},
            .imageExtent = to_vulkan_extent(cmd_data->dst_image_size),
        };

        vkCmdCopyBufferToImage(
            command_buffer,
            src_buffer,
            dst_image,
            to_vulkan_type(cmd_data->dst_image_layout),
            1u,
            &copy);
    }
} // namespace orion::vulkan
