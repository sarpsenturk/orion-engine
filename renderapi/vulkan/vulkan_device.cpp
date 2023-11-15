#include "vulkan_device.h"

#include "vulkan_command.h"
#include "vulkan_conversion.h"
#include "vulkan_platform.h"
#include "vulkan_swapchain.h"

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
        const auto allocator_info = VmaAllocatorCreateInfo{
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
        std::ranges::transform(attachment_list.input_attachments, iter, to_attachment);

        // Get attachment index offsets
        const auto color_attachment_offset = 0u;
        const auto input_attachment_offset = static_cast<std::uint32_t>(attachment_list.color_attachments.size());

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

        std::vector<VkAttachmentReference> input_attachments(attachment_list.input_attachments.size());
        std::ranges::transform(attachment_list.input_attachments, input_attachments.begin(), to_attachment_ref(input_attachment_offset));

        // Create render pass
        VkRenderPass render_pass = VK_NULL_HANDLE;
        {
            const auto subpass = VkSubpassDescription{
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = static_cast<std::uint32_t>(input_attachments.size()),
                .pInputAttachments = input_attachments.data(),
                .colorAttachmentCount = static_cast<std::uint32_t>(color_attachments.size()),
                .pColorAttachments = color_attachments.data(),
                .pResolveAttachments = nullptr,
                .pDepthStencilAttachment = nullptr,
                .preserveAttachmentCount = 0,
                .pPreserveAttachments = nullptr,
            };
            const auto subpass_dependencies = std::array{
                VkSubpassDependency{
                    .srcSubpass = VK_SUBPASS_EXTERNAL,
                    .dstSubpass = 0,
                    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    .srcAccessMask = 0,
                    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    .dependencyFlags = 0,
                },
            };
            const auto info = VkRenderPassCreateInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .attachmentCount = attachment_count,
                .pAttachments = attachments.data(),
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = static_cast<std::uint32_t>(subpass_dependencies.size()),
                .pDependencies = subpass_dependencies.data(),
            };
            vk_result_check(vkCreateRenderPass(device(), &info, alloc_callbacks(), &render_pass));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkRenderPass {}", fmt::ptr(render_pass));
        }
        return render_pass;
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
            vk_result_check(vkCreateCommandPool(device(), &info, alloc_callbacks(), &command_pool));
        }
        return std::make_unique<VulkanCommandAllocator>(this, unique(command_pool, device()));
    }

    std::unique_ptr<Swapchain> VulkanDevice::create_swapchain_api(const SwapchainDesc& desc)
    {
        // Create surface
        VkSurfaceKHR surface = create_platform_surface(instance_, *desc.window);

        // Create swapchain
        VkSwapchainKHR swapchain = create_vk_swapchain({
            .surface = surface,
            .image_count = desc.image_count,
            .format = to_vulkan_type(desc.image_format),
            .extent = to_vulkan_extent(desc.image_size),
            .usage = to_vulkan_type(desc.image_usage),
            .present_mode = VK_PRESENT_MODE_FIFO_KHR, // TODO: Allow present mode to be changed
        });

        return std::make_unique<VulkanSwapchain>(this, unique(surface, instance_), unique(swapchain, device()));
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
            const auto info = VkShaderModuleCreateInfo{
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

    GPUJobHandle VulkanDevice::create_job_api(const GPUJobDesc& desc)
    {
        // Create fence and semaphore associated with this job
        VkFence fence = create_vk_fence(desc.start_finished);
        VkSemaphore semaphore = create_vk_semaphore();

        // Find semaphore dependencies
        auto find_dep_semaphore = [this](GPUJobHandle job_handle) { return jobs_.at(job_handle).vk_semaphore(); };
        std::vector<VkSemaphore> wait_semaphores(desc.dependencies.size());
        std::ranges::transform(desc.dependencies, wait_semaphores.begin(), find_dep_semaphore);

        const auto handle = GPUJobHandle::generate();
        jobs_.insert(std::make_pair(handle, VulkanJob{unique(fence, device()), unique(semaphore, device()), std::move(wait_semaphores)}));
        return handle;
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

    void VulkanDevice::destroy_api(GPUJobHandle job_handle)
    {
        jobs_.erase(job_handle);
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

    void VulkanDevice::wait_for_job_api(GPUJobHandle job_handle)
    {
        VkFence fence = jobs_.at(job_handle).vk_fence();
        vk_result_check(vkWaitForFences(device(), 1, &fence, VK_TRUE, UINT64_MAX));
    }

    void VulkanDevice::wait_for_jobs_api(std::span<const GPUJobHandle> job_handles)
    {
        std::vector<VkFence> fences{job_handles.size()};
        std::ranges::transform(job_handles, fences.begin(), [this](const auto handle) { return jobs_.at(handle).vk_fence(); });
        vk_result_check(vkWaitForFences(device(), static_cast<std::uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));
    }

    void VulkanDevice::wait_queue_idle_api(CommandQueueType queue_type)
    {
        vk_result_check(vkQueueWaitIdle(get_queue(queue_type)));
    }

    void VulkanDevice::wait_idle_api()
    {
        vk_result_check(vkDeviceWaitIdle(device()));
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
            vk_result_check(vkCreateSemaphore(device(), &info, alloc_callbacks(), &semaphore));
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
            vk_result_check(vkCreateFence(device(), &info, alloc_callbacks(), &fence));
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
            vk_result_check(vkCreateSwapchainKHR(device(), &info, alloc_callbacks(), &swapchain));
            SPDLOG_LOGGER_TRACE(logger(), "Created VkSwapchain {}", fmt::ptr(swapchain));
        }
        return swapchain;
    }
} // namespace orion::vulkan
