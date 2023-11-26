#include "vulkan_device.h"

#include "vulkan_command.h"
#include "vulkan_conversion.h"
#include "vulkan_platform.h"
#include "vulkan_swapchain.h"

#include "orion-utils/assertion.h"
#include "orion-utils/functors.h"
#include "orion-utils/static_vector.h"

#include <array>
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
        , vma_allocator_(create_vma_allocator(instance_, physical_device_))
        , descriptor_pool_(create_descriptor_pool())
        , resource_manager_(vk_device(), vma_allocator())
    {
        create_vma_allocator(instance, physical_device);
    }

    UniqueVmaAllocator VulkanDevice::create_vma_allocator(VkInstance instance, VkPhysicalDevice physical_device) const
    {
        const auto allocator_info = VmaAllocatorCreateInfo{
            .flags = 0,
            .physicalDevice = physical_device,
            .device = vk_device(),
            .preferredLargeHeapBlockSize = 0,
            .pHeapSizeLimit = nullptr,
            .pVulkanFunctions = nullptr,
            .instance = instance,
            .vulkanApiVersion = vulkan_api_version,
            .pTypeExternalMemoryHandleTypes = nullptr,
        };
        VmaAllocator allocator = VK_NULL_HANDLE;
        vk_result_check(vmaCreateAllocator(&allocator_info, &allocator));
        return unique(allocator);
    }

    UniqueVkDescriptorPool VulkanDevice::create_descriptor_pool() const
    {
        // TODO: Come up with a better solution to descriptor pools
        //  Descriptor pool per descriptor layout?

        constexpr auto sampler_count = 1000u;
        constexpr auto combined_image_sampler_count = 1000u;
        constexpr auto sampled_image_count = 1000u;
        constexpr auto storage_image_count = 1000u;
        constexpr auto uniform_texel_buffer_count = 1000u;
        constexpr auto storage_texel_buffer_count = 1000u;
        constexpr auto uniform_buffer_count = 1000u;
        constexpr auto storage_buffer_count = 1000u;
        constexpr auto uniform_buffer_dynamic_count = 1000u;
        constexpr auto storage_buffer_dynamic_count = 1000u;
        constexpr auto input_attachment_count = 1000u;
        constexpr auto pool_sizes = std::array{
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, sampler_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combined_image_sampler_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, sampled_image_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, storage_image_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, uniform_texel_buffer_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, storage_texel_buffer_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniform_buffer_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, storage_buffer_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, uniform_buffer_dynamic_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, storage_buffer_dynamic_count},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, input_attachment_count},
        };

        constexpr auto max_sets = 1000u;
        VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
        {
            const auto info = VkDescriptorPoolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                .maxSets = max_sets,
            };
            vk_result_check(vkCreateDescriptorPool(vk_device(), &info, alloc_callbacks(), &descriptor_pool));
        }
        return unique(descriptor_pool, vk_device());
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
                return queues_.transfer.family;
            case CommandQueueType::Compute:
                return queues_.compute.family;
            case CommandQueueType::Graphics:
                [[fallthrough]];
            case CommandQueueType::Any:
                return queues_.graphics.family;
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

    VkDescriptorSetLayout VulkanDevice::create_vk_descriptor_set_layout(const DescriptorLayoutDesc& desc) const
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings(desc.bindings.size());
        std::ranges::transform(desc.bindings, bindings.begin(), [index = 0u](const DescriptorBindingDesc& binding) mutable {
            return VkDescriptorSetLayoutBinding{
                .binding = index++,
                .descriptorType = to_vulkan_type(binding.type),
                .descriptorCount = binding.count,
                .stageFlags = to_vulkan_type(binding.shader_stages),
                .pImmutableSamplers = nullptr,
            };
        });

        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
        {
            const auto info = VkDescriptorSetLayoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings = bindings.data(),
            };
            vk_result_check(vkCreateDescriptorSetLayout(vk_device(), &info, alloc_callbacks(), &descriptor_set_layout));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkDescriptorSetLayout {}", fmt::ptr(descriptor_set_layout));
        }
        return descriptor_set_layout;
    }

    VkDescriptorSet VulkanDevice::create_vk_descriptor_set(VkDescriptorSetLayout descriptor_set_layout) const
    {
        VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
        {
            const auto info = VkDescriptorSetAllocateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = descriptor_pool_.get(),
                .descriptorSetCount = 1,
                .pSetLayouts = &descriptor_set_layout,
            };
            vk_result_check(vkAllocateDescriptorSets(vk_device(), &info, &descriptor_set));
            SPDLOG_LOGGER_TRACE(logger(), "Allocated VkDescriptorSet {}", fmt::ptr(descriptor_set));
        }
        return descriptor_set;
    }

    std::unique_ptr<CommandAllocator> VulkanDevice::create_command_allocator_api(CommandQueueType queue_type)
    {
        VkCommandPool command_pool = VK_NULL_HANDLE;
        {
            const auto info = VkCommandPoolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = get_queue_family(queue_type),
            };
            vk_result_check(vkCreateCommandPool(vk_device(), &info, alloc_callbacks(), &command_pool));
        }
        return std::make_unique<VulkanCommandAllocator>(this, unique(command_pool, vk_device()));
    }

    std::unique_ptr<Swapchain> VulkanDevice::create_swapchain_api(Window& window, const SwapchainDesc& desc)
    {
        // Create surface
        VkSurfaceKHR surface = create_platform_surface(instance_, window);

        // Create swapchain
        VkSwapchainKHR swapchain = create_vk_swapchain({
            .surface = surface,
            .image_count = desc.image_count,
            .format = to_vulkan_type(desc.image_format),
            .extent = to_vulkan_extent(desc.image_size),
            .usage = to_vulkan_type(desc.image_usage),
            .present_mode = VK_PRESENT_MODE_FIFO_KHR, // TODO: Allow present mode to be changed
        });

        return std::make_unique<VulkanSwapchain>(this, unique(surface, instance_), unique(swapchain, vk_device()));
    }

    RenderPassHandle VulkanDevice::create_render_pass_api(const RenderPassDesc& desc)
    {
        VkRenderPass render_pass = VK_NULL_HANDLE;
        {
            std::vector<VkAttachmentDescription> attachments;
            attachments.reserve(desc.attachment_count());
            auto make_attachment = [&, index = 0u](const AttachmentDesc& attachment) mutable {
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
                return VkAttachmentReference{
                    .attachment = index++,
                    .layout = to_vulkan_type(attachment.layout),
                };
            };

            std::vector<VkAttachmentReference> input_attachments(desc.input_attachments.size());
            std::ranges::transform(desc.input_attachments, input_attachments.begin(), make_attachment);
            std::vector<VkAttachmentReference> color_attachments(desc.color_attachments.size());
            std::ranges::transform(desc.color_attachments, color_attachments.begin(), make_attachment);

            const auto subpass = VkSubpassDescription{
                .flags = 0,
                .pipelineBindPoint = to_vulkan_type(desc.bind_point),
                .inputAttachmentCount = static_cast<uint32_t>(input_attachments.size()),
                .pInputAttachments = input_attachments.data(),
                .colorAttachmentCount = static_cast<uint32_t>(color_attachments.size()),
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
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = 1u,
                .pSubpasses = &subpass,
                .dependencyCount = 0,
                .pDependencies = nullptr,
            };
            vk_result_check(vkCreateRenderPass(vk_device(), &info, alloc_callbacks(), &render_pass));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkRenderPass {}", fmt::ptr(render_pass));
        }

        auto handle = RenderPassHandle::generate();
        resource_manager_.add(handle, render_pass);
        return handle;
    }

    FramebufferHandle VulkanDevice::create_framebuffer_api(const FramebufferDesc& desc)
    {
        // Find image views
        std::vector<VkImageView> image_views(desc.image_views.size());
        std::ranges::transform(desc.image_views, image_views.begin(), [this](auto handle) {
            return resource_manager_.find(handle);
        });

        // Create framebuffer
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        {
            const auto info = VkFramebufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = resource_manager_.find(desc.render_pass),
                .attachmentCount = static_cast<std::uint32_t>(image_views.size()),
                .pAttachments = image_views.data(),
                .width = desc.size.x(),
                .height = desc.size.y(),
                .layers = 1,
            };
            vk_result_check(vkCreateFramebuffer(vk_device(), &info, alloc_callbacks(), &framebuffer));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkFramebuffer {}", fmt::ptr(framebuffer));
        }

        const auto handle = FramebufferHandle::generate();
        resource_manager_.add(handle, framebuffer);
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
            const auto info = VkShaderModuleCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .codeSize = desc.byte_code.size_bytes(),
                .pCode = spirv.data(),
            };
            vk_result_check(vkCreateShaderModule(vk_device(), &info, alloc_callbacks(), &shader_module));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkShaderModule {}", fmt::ptr(shader_module));
        }

        auto handle = ShaderModuleHandle::generate();
        resource_manager_.add(handle, shader_module);
        return handle;
    }

    DescriptorLayoutHandle VulkanDevice::create_descriptor_layout_api(const DescriptorLayoutDesc& desc)
    {
        const auto handle = DescriptorLayoutHandle{desc.hash()};
        if (resource_manager_.find(handle) != VK_NULL_HANDLE) {
            return handle;
        }

        VkDescriptorSetLayout descriptor_set_layout = create_vk_descriptor_set_layout(desc);
        resource_manager_.add(handle, descriptor_set_layout);
        return handle;
    }

    DescriptorHandle VulkanDevice::create_descriptor_api(DescriptorLayoutHandle descriptor_layout_handle)
    {
        VkDescriptorSetLayout layout = resource_manager_.find(descriptor_layout_handle);
        VkDescriptorSet descriptor_set = create_vk_descriptor_set(layout);
        const auto handle = DescriptorHandle::generate();
        resource_manager_.add(handle, descriptor_set, descriptor_pool_.get());
        return handle;
    }

    PipelineLayoutHandle VulkanDevice::create_pipeline_layout_api(const PipelineLayoutDesc& desc)
    {
        std::vector<VkDescriptorSetLayout> descriptor_sets(desc.descriptors.size());
        std::ranges::transform(desc.descriptors, descriptor_sets.begin(), [this](DescriptorLayoutHandle descriptor_layout) {
            return resource_manager_.find(descriptor_layout);
        });

        std::vector<VkPushConstantRange> push_constants(desc.push_constants.size());
        std::ranges::transform(desc.push_constants, push_constants.begin(), [offset = 0u](const PushConstantDesc& push_constant) mutable {
            return VkPushConstantRange{
                .stageFlags = to_vulkan_type(push_constant.shader_stages),
                .offset = (offset += push_constant.size),
                .size = push_constant.size,
            };
        });

        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        {
            const auto info = VkPipelineLayoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = static_cast<std::uint32_t>(descriptor_sets.size()),
                .pSetLayouts = descriptor_sets.data(),
                .pushConstantRangeCount = static_cast<uint32_t>(push_constants.size()),
                .pPushConstantRanges = push_constants.data(),
            };
            vk_result_check(vkCreatePipelineLayout(vk_device(), &info, alloc_callbacks(), &pipeline_layout));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkPipelineLayout {}", fmt::ptr(pipeline_layout));
        }

        const auto handle = PipelineLayoutHandle::generate();
        resource_manager_.add(handle, pipeline_layout);
        return handle;
    }

    PipelineHandle VulkanDevice::create_graphics_pipeline_api(const GraphicsPipelineDesc& desc)
    {
        // Create pipeline layout
        VkPipelineLayout pipeline_layout = resource_manager_.find(desc.pipeline_layout);

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
                    .module = resource_manager_.find(shader.module),
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

        // Create VkPipeline
        VkPipeline pipeline = VK_NULL_HANDLE;
        {
            const auto info = VkGraphicsPipelineCreateInfo{
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
                .renderPass = resource_manager_.find(desc.render_pass),
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE,
                .basePipelineIndex = 0,
            };
            vk_result_check(vkCreateGraphicsPipelines(vk_device(), VK_NULL_HANDLE, 1, &info, alloc_callbacks(), &pipeline));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkPipeline {}", fmt::ptr(pipeline));
        }

        const auto handle = PipelineHandle::generate();
        resource_manager_.add(handle, pipeline);
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
        resource_manager_.add(handle, buffer, allocation);
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
        resource_manager_.add(handle, image, allocation);
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
                .image = resource_manager_.find(desc.image).image,
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
            vk_result_check(vkCreateImageView(vk_device(), &info, alloc_callbacks(), &image_view));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkImageView {}", fmt::ptr(image_view));
        }
        const auto handle = ImageViewHandle::generate();
        resource_manager_.add(handle, image_view);
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
            vk_result_check(vkCreateSampler(vk_device(), &info, alloc_callbacks(), &sampler));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkSampler {}", fmt::ptr(sampler));
        }
        const auto handle = SamplerHandle::generate();
        resource_manager_.add(handle, sampler);
        return handle;
    }

    FenceHandle VulkanDevice::create_fence_api(const FenceDesc& desc)
    {
        const auto handle = FenceHandle::generate();
        resource_manager_.add(handle, create_vk_fence(desc.start_finished));
        return handle;
    }

    SemaphoreHandle VulkanDevice::create_semaphore_api()
    {
        const auto handle = SemaphoreHandle::generate();
        resource_manager_.add(handle, create_vk_semaphore());
        return handle;
    }

    void VulkanDevice::destroy_api(RenderPassHandle render_pass_handle)
    {
        resource_manager_.remove(render_pass_handle);
    }

    void VulkanDevice::destroy_api(FramebufferHandle framebuffer_handle)
    {
        resource_manager_.remove(framebuffer_handle);
    }

    void VulkanDevice::destroy_api(ShaderModuleHandle shader_module_handle)
    {
        resource_manager_.remove(shader_module_handle);
    }

    void VulkanDevice::destroy_api(DescriptorLayoutHandle descriptor_layout_handle)
    {
        resource_manager_.remove(descriptor_layout_handle);
    }

    void VulkanDevice::destroy_api(DescriptorHandle descriptor_handle)
    {
        resource_manager_.remove(descriptor_handle);
    }

    void VulkanDevice::destroy_api(PipelineLayoutHandle pipeline_layout_handle)
    {
        resource_manager_.remove(pipeline_layout_handle);
    }

    void VulkanDevice::destroy_api(PipelineHandle graphics_pipeline_handle)
    {
        resource_manager_.remove(graphics_pipeline_handle);
    }

    void VulkanDevice::destroy_api(GPUBufferHandle buffer_handle)
    {
        resource_manager_.remove(buffer_handle);
    }

    void VulkanDevice::destroy_api(ImageHandle image_handle)
    {
        resource_manager_.remove(image_handle);
    }

    void VulkanDevice::destroy_api(ImageViewHandle image_view_handle)
    {
        resource_manager_.remove(image_view_handle);
    }

    void VulkanDevice::destroy_api(SamplerHandle sampler_handle)
    {
        resource_manager_.remove(sampler_handle);
    }

    void VulkanDevice::destroy_api(FenceHandle fence_handle)
    {
        resource_manager_.remove(fence_handle);
    }

    void VulkanDevice::destroy_api(SemaphoreHandle semaphore_handle)
    {
        resource_manager_.remove(semaphore_handle);
    }

    void* VulkanDevice::map_api(GPUBufferHandle buffer_handle)
    {
        void* ptr = nullptr;
        VmaAllocation allocation = resource_manager_.find(buffer_handle).allocation;
        vk_result_check(vmaMapMemory(vma_allocator_.get(), allocation, &ptr));
        SPDLOG_LOGGER_TRACE(logger(), "Mapped VmaAllocation {} at memory address {}", fmt::ptr(allocation), fmt::ptr(ptr));
        return ptr;
    }

    void VulkanDevice::unmap_api(GPUBufferHandle buffer_handle)
    {
        vmaUnmapMemory(vma_allocator_.get(), resource_manager_.find(buffer_handle).allocation);
    }

    void VulkanDevice::wait_for_fences_api(std::span<const FenceHandle> fence_handles)
    {
        std::vector<VkFence> fences{fence_handles.size()};
        std::ranges::transform(fence_handles, fences.begin(), [this](const auto handle) { return resource_manager_.find(handle); });
        vk_result_check(vkWaitForFences(vk_device(), static_cast<std::uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));
        vk_result_check(vkResetFences(vk_device(), static_cast<std::uint32_t>(fences.size()), fences.data()));
    }

    void VulkanDevice::wait_queue_idle_api(CommandQueueType queue_type)
    {
        vk_result_check(vkQueueWaitIdle(get_queue(queue_type)));
    }

    void VulkanDevice::wait_idle_api()
    {
        vk_result_check(vkDeviceWaitIdle(vk_device()));
    }

    void VulkanDevice::write_descriptor_api(DescriptorHandle descriptor_handle, std::span<const DescriptorBinding> bindings)
    {
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(bindings.size());
        std::vector<VkDescriptorBufferInfo> buffer_descriptors;
        buffer_descriptors.reserve(std::ranges::count_if(bindings, is_buffer_binding));

        VkDescriptorSet descriptor_set = resource_manager_.find(descriptor_handle);
        std::ranges::for_each(bindings, [&](const DescriptorBinding& binding) {
            VkWriteDescriptorSet write_descriptor{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_set,
                .dstBinding = binding.binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = to_vulkan_type(binding.binding_type),
                .pImageInfo = nullptr,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr,
            };
            if (const auto& buffer = binding.buffer; binding.is_buffer()) {
                buffer_descriptors.push_back({
                    .buffer = resource_manager_.find(buffer.buffer_handle).buffer,
                    .offset = buffer.offset,
                    .range = buffer.size,
                });
                write_descriptor.pBufferInfo = &(buffer_descriptors.back());
            }
            writes.push_back(write_descriptor);
        });

        vkUpdateDescriptorSets(vk_device(), static_cast<uint32_t>(writes.size()), writes.data(), 0u, nullptr);
    }

    void VulkanDevice::submit_api(const SubmitDesc& desc)
    {
        std::vector<VkSemaphore> wait_semaphores(desc.wait_semaphores.size());
        std::ranges::transform(desc.wait_semaphores, wait_semaphores.begin(), [this](auto handle) { return resource_manager_.find(handle); });
        std::vector<VkPipelineStageFlags> wait_stages(desc.wait_semaphores.size(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        std::vector<VkCommandBuffer> command_buffers(desc.command_lists.size());
        std::ranges::transform(desc.command_lists | std::views::transform(StaticCast<const VulkanCommandList*>{}), command_buffers.begin(), &VulkanCommandList::vk_command_buffer);

        const auto submit = VkSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size()),
            .pWaitSemaphores = wait_semaphores.data(),
            .pWaitDstStageMask = wait_stages.data(),
            .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
            .pCommandBuffers = command_buffers.data(),
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
        };
        vk_result_check(vkQueueSubmit(get_queue(desc.queue_type), 1u, &submit, resource_manager_.find(desc.signal_fence)));
    }

    VkSemaphore VulkanDevice::create_vk_semaphore()
    {
        VkSemaphore semaphore = VK_NULL_HANDLE;
        {
            const auto info = VkSemaphoreCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
            };
            vk_result_check(vkCreateSemaphore(vk_device(), &info, alloc_callbacks(), &semaphore));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkSemaphore {}", fmt::ptr(semaphore));
        }
        return semaphore;
    }

    VkFence VulkanDevice::create_vk_fence(bool signaled)
    {
        VkFence fence = VK_NULL_HANDLE;
        {
            const auto info = VkFenceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags{},
            };
            vk_result_check(vkCreateFence(vk_device(), &info, alloc_callbacks(), &fence));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkFence {}", fmt::ptr(fence));
        }
        return fence;
    }

    VkSwapchainKHR VulkanDevice::create_vk_swapchain(const VulkanSwapchainDesc& desc)
    {
        // Get surface capabilities
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vk_result_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, desc.surface, &surface_capabilities));
        // Create swapchain
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        {
            const auto info = VkSwapchainCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = desc.surface,
                .minImageCount = desc.image_count,
                .imageFormat = desc.format,
                .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
                .imageExtent = desc.extent,
                .imageArrayLayers = 1,
                .imageUsage = desc.usage,
                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .preTransform = surface_capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = desc.present_mode,
                .clipped = VK_TRUE,
                .oldSwapchain = desc.old_swapchain,
            };
            vk_result_check(vkCreateSwapchainKHR(vk_device(), &info, alloc_callbacks(), &swapchain));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkSwapchain {}", fmt::ptr(swapchain));
        }
        return swapchain;
    }
} // namespace orion::vulkan
