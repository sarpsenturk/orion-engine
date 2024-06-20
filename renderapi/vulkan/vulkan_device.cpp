#include "vulkan_device.h"

#include "vulkan_command.h"
#include "vulkan_conversion.h"
#include "vulkan_platform.h"
#include "vulkan_queue.h"
#include "vulkan_reflection.h"
#include "vulkan_renderpass.h"
#include "vulkan_swapchain.h"

#include "orion-utils/assertion.h"
#include "orion-utils/static_vector.h"

#include <array>
#include <ranges>
#include <utility>

#include <spdlog/spdlog.h>

namespace orion::vulkan
{
    namespace
    {
        void* map_allocation(VmaAllocator allocator, VmaAllocation allocation)
        {
            void* ptr;
            vk_result_check(vmaMapMemory(allocator, allocation, &ptr));
            return ptr;
        }
    } // namespace

    VulkanDevice::VulkanDevice(spdlog::logger* logger, VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues)
        : RenderDevice(logger)
        , instance_(instance)
        , physical_device_(physical_device)
        , device_(std::move(device))
        , queues_(queues)
        , vma_allocator_(create_vma_allocator(instance_, physical_device_))
        , resource_manager_(vk_device(), vma_allocator())
    {
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

    std::unique_ptr<CommandQueue> VulkanDevice::create_queue_api(CommandQueueType type)
    {
        return std::make_unique<VulkanQueue>(vk_device(), get_queue(type), get_queue_family(type), resource_manager());
    }

    std::unique_ptr<CommandAllocator> VulkanDevice::create_command_allocator_api(const CommandAllocatorDesc& desc)
    {
        VkCommandPool command_pool = VK_NULL_HANDLE;
        {
            const auto info = VkCommandPoolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = desc.reset_command_buffer ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : VkCommandPoolCreateFlags{},
                .queueFamilyIndex = get_queue_family(desc.queue_type),
            };
            vk_result_check(vkCreateCommandPool(vk_device(), &info, alloc_callbacks(), &command_pool));
        }
        return std::make_unique<VulkanCommandAllocator>(vk_device(), resource_manager(), unique(command_pool, vk_device()));
    }

    std::unique_ptr<Swapchain> VulkanDevice::create_swapchain_api(CommandQueue* queue, const Window& window, const SwapchainDesc& desc)
    {
        // Create surface
        VkSurfaceKHR surface = create_platform_surface(instance_, window);
        const auto info = VkSwapchainCreateInfoKHR{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = surface,
            .minImageCount = desc.image_count,
            .imageFormat = to_vulkan_format(desc.image_format),
            .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = VkExtent2D{desc.width, desc.height},
            .imageArrayLayers = 1,
            .imageUsage = to_vulkan_type(desc.image_usage),
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            // Create initial swapchain with PRESENT_MODE_FIFO.
            // This may lead to a recreation of the swapchain the first time swapchain->present() is called
            // but it's just a single frame
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
        };
        return std::make_unique<VulkanSwapchain>(vk_device(), physical_device_, static_cast<VulkanQueue*>(queue), resource_manager(), unique(surface, instance_), info);
    }

    std::unique_ptr<ShaderReflector> VulkanDevice::create_shader_reflector_api()
    {
        return std::make_unique<VulkanShaderReflector>();
    }

    std::unique_ptr<RenderPass> VulkanDevice::create_render_pass_api()
    {
        return std::make_unique<VulkanRenderPass>(resource_manager());
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

    DescriptorPoolHandle VulkanDevice::create_descriptor_pool_api(const DescriptorPoolDesc& desc)
    {
        VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
        const auto flags = to_vulkan_type(desc.flags);
        {
            std::vector<VkDescriptorPoolSize> pool_sizes(desc.sizes.size());
            std::ranges::transform(desc.sizes, pool_sizes.begin(), [](const DescriptorPoolSize& pool_size) {
                return VkDescriptorPoolSize{
                    .type = to_vulkan_type(pool_size.type),
                    .descriptorCount = pool_size.count,
                };
            });
            const auto info = VkDescriptorPoolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = flags,
                .maxSets = desc.max_descriptors,
                .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
                .pPoolSizes = pool_sizes.data(),
            };
            vk_result_check(vkCreateDescriptorPool(vk_device(), &info, alloc_callbacks(), &descriptor_pool));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkDescriptorPool {}", fmt::ptr(descriptor_pool));
        }
        auto handle = DescriptorPoolHandle::generate();
        resource_manager_.add(handle, descriptor_pool, flags);
        return handle;
    }

    DescriptorHandle VulkanDevice::create_descriptor_api(DescriptorLayoutHandle descriptor_layout_handle, DescriptorPoolHandle descriptor_pool_handle)
    {
        VkDescriptorSetLayout layout = resource_manager_.find(descriptor_layout_handle);
        const auto descriptor_pool = resource_manager_.find(descriptor_pool_handle);
        VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
        {
            const auto info = VkDescriptorSetAllocateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = descriptor_pool.descriptor_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &layout,
            };
            vk_result_check(vkAllocateDescriptorSets(vk_device(), &info, &descriptor_set));
            SPDLOG_LOGGER_TRACE(logger(), "Allocated VkDescriptorSet {}", fmt::ptr(descriptor_set));
        }
        const auto handle = DescriptorHandle::generate();
        resource_manager_.add(handle, descriptor_set, descriptor_pool.descriptor_pool, (descriptor_pool.flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));
        return handle;
    }

    PipelineLayoutHandle VulkanDevice::create_pipeline_layout_api(const PipelineLayoutDesc& desc)
    {
        const auto descriptors = resource_manager_.find(desc.descriptors);

        std::vector<VkPushConstantRange> push_constants(desc.push_constants.size());
        std::ranges::transform(desc.push_constants, push_constants.begin(), [offset = 0u](const PushConstantDesc& push_constant) mutable {
            const auto current_offset = offset;
            offset += push_constant.size;
            return VkPushConstantRange{
                .stageFlags = to_vulkan_type(push_constant.shader_stages),
                .offset = current_offset,
                .size = push_constant.size,
            };
        });

        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        {
            const auto info = VkPipelineLayoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = static_cast<std::uint32_t>(descriptors.size()),
                .pSetLayouts = descriptors.data(),
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
        ORION_EXPECTS(pipeline_layout != VK_NULL_HANDLE);

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

        const auto vk_vertex_attributes = [attributes = desc.vertex_attributes]() {
            std::vector<VkVertexInputAttributeDescription> vk_attributes(attributes.size());
            std::ranges::transform(attributes, vk_attributes.begin(), [offset = 0u, location = 0u](const VertexAttributeDesc& attribute) mutable {
                const auto attr_offset = offset;
                offset += format_size(attribute.format);
                return VkVertexInputAttributeDescription{
                    .location = location++,
                    .binding = 0,
                    .format = to_vulkan_format(attribute.format),
                    .offset = attr_offset,
                };
            });
            return vk_attributes;
        }();

        const auto vk_vertex_binding = [stride = vertex_input_stride(desc.vertex_attributes)]() {
            std::vector<VkVertexInputBindingDescription> vk_bindings;
            if (stride != 0) {
                vk_bindings.push_back({
                    .binding = 0,
                    .stride = stride,
                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                });
            }
            return vk_bindings;
        }();

        const auto vk_input_state = VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = static_cast<std::uint32_t>(vk_vertex_binding.size()),
            .pVertexBindingDescriptions = vk_vertex_binding.data(),
            .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vk_vertex_attributes.size()),
            .pVertexAttributeDescriptions = vk_vertex_attributes.data(),
        };

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

        const auto vk_render_targets = [render_targets = desc.render_targets]() {
            std::vector<VkFormat> vk_render_targets(render_targets.size());
            std::ranges::transform(render_targets, vk_render_targets.begin(), to_vulkan_format);
            return vk_render_targets;
        }();
        const auto vk_rendering_info = VkPipelineRenderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .pNext = nullptr,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<uint32_t>(vk_render_targets.size()),
            .pColorAttachmentFormats = vk_render_targets.data(),
            .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
            .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
        };

        // Create VkPipeline
        VkPipeline pipeline = VK_NULL_HANDLE;
        {
            const auto info = VkGraphicsPipelineCreateInfo{
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &vk_rendering_info,
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
                .renderPass = VK_NULL_HANDLE,
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
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
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
            const auto image_info = VkImageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .imageType = to_vulkan_type(desc.type),
                .format = to_vulkan_format(desc.format),
                .extent = to_vulkan_extent(desc.size),
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = to_vulkan_type(desc.tiling),
                .usage = to_vulkan_type(desc.usage),
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
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
                .format = to_vulkan_format(desc.format),
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
                .mipLodBias = desc.mip_lod_bias,
                .anisotropyEnable = VK_FALSE, // TODO: Make this customizable
                .maxAnisotropy = desc.max_anisotropy,
                .compareEnable = VK_FALSE,
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

    void VulkanDevice::destroy_api(ShaderModuleHandle shader_module_handle)
    {
        resource_manager_.remove(shader_module_handle);
    }

    void VulkanDevice::destroy_api(DescriptorLayoutHandle descriptor_layout_handle)
    {
        resource_manager_.remove(descriptor_layout_handle);
    }

    void VulkanDevice::destroy_api(DescriptorPoolHandle descriptor_pool_handle)
    {
        resource_manager_.remove(descriptor_pool_handle);
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

    void VulkanDevice::destroy_flush_api()
    {
        resource_manager_.destroy_flush();
    }

    void* VulkanDevice::map_api(GPUBufferHandle buffer_handle)
    {
        return map_allocation(vma_allocator(), resource_manager_.find(buffer_handle).allocation);
    }

    void* VulkanDevice::map_api(ImageHandle image_handle)
    {
        return map_allocation(vma_allocator(), resource_manager_.find(image_handle).allocation);
    }

    void VulkanDevice::unmap_api(GPUBufferHandle buffer_handle)
    {
        vmaUnmapMemory(vma_allocator(), resource_manager_.find(buffer_handle).allocation);
    }

    void VulkanDevice::unmap_api(ImageHandle image_handle)
    {
        vmaUnmapMemory(vma_allocator(), resource_manager_.find(image_handle).allocation);
    }

    void VulkanDevice::wait_for_fences_api(std::span<const FenceHandle> fence_handles)
    {
        const auto fences = resource_manager_.find(fence_handles);
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

    void VulkanDevice::reset_descriptor_pool_api(DescriptorPoolHandle descriptor_pool_handle)
    {
        VkDescriptorPool descriptor_pool = resource_manager_.find(descriptor_pool_handle).descriptor_pool;
        vk_result_check(vkResetDescriptorPool(vk_device(), descriptor_pool, 0));
    }

    void VulkanDevice::write_descriptor_api(DescriptorHandle descriptor_handle, std::span<const DescriptorWrite> writes)
    {
        VkDescriptorSet descriptor_set = resource_manager_.find(descriptor_handle);
        ORION_ASSERT(descriptor_set != VK_NULL_HANDLE);

        struct VulkanDescriptorWriteInfo {
            std::vector<VkDescriptorImageInfo> images;
            std::vector<VkDescriptorBufferInfo> buffers;
        };

        auto to_vk_image_info = [this](const ImageDescriptorDesc& image_descriptor) {
            return VkDescriptorImageInfo{
                .sampler = resource_manager_.find(image_descriptor.sampler_handle),
                .imageView = resource_manager_.find(image_descriptor.image_view_handle),
                .imageLayout = to_vulkan_type(image_descriptor.image_layout),
            };
        };
        auto to_vk_buffer_info = [this](const BufferDescriptorDesc& buffer_descriptor) {
            return VkDescriptorBufferInfo{
                .buffer = resource_manager_.find(buffer_descriptor.buffer_handle).buffer,
                .offset = buffer_descriptor.region.offset,
                .range = buffer_descriptor.region.size,
            };
        };

        std::vector<VkWriteDescriptorSet> vk_writes(writes.size());
        std::vector<VulkanDescriptorWriteInfo> vk_write_infos(writes.size());
        for (int i = 0; i < writes.size(); ++i) {
            vk_writes[i] = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_set,
                .dstBinding = writes[i].binding,
                .dstArrayElement = writes[i].array_start,
                .descriptorType = to_vulkan_type(writes[i].descriptor_type),
            };

            if (!writes[i].buffers.empty()) {
                vk_write_infos[i].buffers.resize(writes[i].buffers.size());
                std::ranges::transform(writes[i].buffers, vk_write_infos[i].buffers.begin(), to_vk_buffer_info);
                vk_writes[i].descriptorCount = static_cast<std::uint32_t>(writes[i].buffers.size());
                vk_writes[i].pBufferInfo = vk_write_infos[i].buffers.data();
            } else if (!writes[i].images.empty()) {
                vk_write_infos[i].images.resize(writes[i].images.size());
                std::ranges::transform(writes[i].images, vk_write_infos[i].images.begin(), to_vk_image_info);
                vk_writes[i].descriptorCount = static_cast<std::uint32_t>(writes[i].images.size());
                vk_writes[i].pImageInfo = vk_write_infos[i].images.data();
            } else {
                unreachable();
            }
        }

        vkUpdateDescriptorSets(
            vk_device(),                             // device
            static_cast<uint32_t>(vk_writes.size()), // VkWriteDescriptorSet count
            vk_writes.data(),                        // VkWriteDescriptorSet pointer
            0u,                                      // VkCopyDescriptorSet count
            nullptr                                  // VkCopyDescriptorSet pointer
        );
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
} // namespace orion::vulkan
