#include "vulkan_device.h"

#include "vulkan_command.h"
#include "vulkan_conversion.h"
#include "vulkan_error.h"
#include "vulkan_platform.h"
#include "vulkan_queue.h"
#include "vulkan_shader.h"
#include "vulkan_swapchain.h"

#include "orion/assertion.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <bit>
#include <stdexcept>
#include <vector>

namespace orion
{
    VulkanDevice::VulkanDevice(VkDevice device, VkInstance instance, VkPhysicalDevice physical_device, VkQueue queue, std::uint32_t queue_family_index)
        : device_(device)
        , instance_(instance)
        , physical_device_(physical_device)
        , queue_(queue)
        , queue_family_index_(queue_family_index)
        , pipeline_layouts_(device_.get())
        , pipelines_(device_.get())
    {
    }

    std::unique_ptr<CommandQueue> VulkanDevice::create_command_queue_api()
    {
        return std::make_unique<VulkanQueue>(queue_);
    }

    std::unique_ptr<Swapchain> VulkanDevice::create_swapchain_api(const SwapchainDesc& desc)
    {
        ORION_EXPECTS(desc.width != 0);
        ORION_EXPECTS(desc.height != 0);

        // Create the surface
        VkSurfaceKHR surface = create_platform_surface(instance_, desc.window);
        SPDLOG_TRACE("Created VkSurfaceKHR {}", fmt::ptr(surface));

        // Check if surface is supported
        {
            VkBool32 surface_supported = VK_FALSE;
            vk_assert(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, queue_family_index_, surface, &surface_supported));
            if (!surface_supported) {
                throw std::runtime_error("Vulkan: physical device queue does not support surface");
            }
        }

        // Get surface capabilities
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vk_assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface, &surface_capabilities));

        // Check requested swapchain against capabilities
        if (desc.image_count > surface_capabilities.maxImageCount) {
            throw std::runtime_error("Vulkan: requested swapchain image count is greater than VkSurfaceCapabilitiesKHR::maxImageCount");
        }

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        {
            const auto info = VkSwapchainCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = surface,
                .minImageCount = desc.image_count,
                .imageFormat = to_vk_format(desc.image_format),
                .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                .imageExtent = {.width = desc.width, .height = desc.height},
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
                .pQueueFamilyIndices = nullptr,
                .preTransform = surface_capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = VK_PRESENT_MODE_FIFO_KHR,
                .clipped = VK_TRUE,
                .oldSwapchain = VK_NULL_HANDLE,
            };
            vk_assert(vkCreateSwapchainKHR(device_.get(), &info, nullptr, &swapchain));
            SPDLOG_TRACE("Created VkSwapchainKHR {}", fmt::ptr(swapchain));
        }
        return std::make_unique<VulkanSwapchain>(device_.get(), instance_, surface, swapchain);
    }

    std::unique_ptr<ShaderCompiler> VulkanDevice::create_shader_compiler_api()
    {
        return std::make_unique<VulkanShaderCompiler>();
    }

    std::unique_ptr<CommandAllocator> VulkanDevice::create_command_allocator_api([[maybe_unused]] const CommandAllocatorDesc& desc)
    {
        VkCommandPool command_pool = VK_NULL_HANDLE;
        {
            const auto info = VkCommandPoolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = queue_family_index_,
            };
            vk_assert(vkCreateCommandPool(device_.get(), &info, nullptr, &command_pool));
            SPDLOG_TRACE("Created VkCommandPool {}", fmt::ptr(command_pool));
        }
        return std::make_unique<VulkanCommandAllocator>(device_.get(), command_pool);
    }

    std::unique_ptr<CommandList> VulkanDevice::create_command_list_api(const CommandListDesc& desc)
    {
        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        {
            const auto* command_pool = dynamic_cast<const VulkanCommandAllocator*>(desc.command_allocator);
            ORION_ASSERT(command_pool != nullptr);
            const auto info = VkCommandBufferAllocateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = command_pool->vk_command_pool(),
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            vk_assert(vkAllocateCommandBuffers(device_.get(), &info, &command_buffer));
            SPDLOG_TRACE("Created VkCommandBuffer {}", fmt::ptr(command_buffer));
        }
        return std::make_unique<VulkanCommandList>(device_.get(), command_buffer);
    }

    PipelineLayoutHandle VulkanDevice::create_pipeline_layout_api([[maybe_unused]] const PipelineLayoutDesc& desc)
    {
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        {
            const auto info = VkPipelineLayoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = 0,
                .pSetLayouts = nullptr,
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr,
            };
            vk_assert(vkCreatePipelineLayout(device_.get(), &info, nullptr, &pipeline_layout));
            SPDLOG_TRACE("Created VkPipelineLayout {}", fmt::ptr(pipeline_layout));
        }
        return pipeline_layouts_.insert(pipeline_layout);
    }

    PipelineHandle VulkanDevice::create_graphics_pipeline_api(const GraphicsPipelineDesc& desc)
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        {
            // Create shader modules
            const auto shader_modules = std::array{
                create_vk_shader_module(desc.vertex_shader),
                create_vk_shader_module(desc.pixel_shader),
            };

            // Pipeline shader stages
            const auto shaders = std::array{
                VkPipelineShaderStageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = shader_modules[0],
                    .pName = "main",
                    .pSpecializationInfo = nullptr,
                },
                VkPipelineShaderStageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = shader_modules[1],
                    .pName = "main",
                    .pSpecializationInfo = nullptr,
                },
            };

            // Pipeline vertex attributes
            std::uint32_t offset = 0;
            std::vector<VkVertexInputAttributeDescription> vertex_attributes(desc.vertex_attributes.size());
            std::ranges::transform(desc.vertex_attributes, vertex_attributes.begin(), [binding = 0u, &offset](const VertexAttribute& attribute) mutable {
                return VkVertexInputAttributeDescription{
                    .location = 0,
                    .binding = binding++,
                    .format = to_vk_format(attribute.format),
                    .offset = offset += static_cast<uint16_t>(format_byte_size(attribute.format)),
                };
            });

            // Pipeline vertex binding
            const auto vertex_binding = VkVertexInputBindingDescription{
                .binding = 0,
                .stride = offset,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            };

            // Pipeline vertex input
            const auto vertex_input = VkPipelineVertexInputStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &vertex_binding,
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes.size()),
                .pVertexAttributeDescriptions = vertex_attributes.data(),
            };

            // Pipeline input assembly
            const auto input_assembly = VkPipelineInputAssemblyStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .topology = to_vk_primitive_topology(desc.primitive_topology),
                .primitiveRestartEnable = VK_FALSE,
            };

            // Pipeline viewport
            const auto viewport = VkPipelineViewportStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .viewportCount = 1,
                .pViewports = nullptr,
                .scissorCount = 1,
                .pScissors = nullptr,
            };

            const auto rasterization = VkPipelineRasterizationStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = to_vk_polygon_mode(desc.rasterizer.fill_mode),
                .cullMode = to_vk_cull_mode(desc.rasterizer.cull_mode),
                .frontFace = desc.rasterizer.front_face == FrontFace::ClockWise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 1.f,
                .depthBiasClamp = 0.f,
                .depthBiasSlopeFactor = 1.f,
                .lineWidth = 1.f,
            };

            const auto multisample = VkPipelineMultisampleStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 0,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE,
            };

            std::vector<VkPipelineColorBlendAttachmentState> blend_attachments(desc.blend.render_targets.size());
            std::ranges::transform(desc.blend.render_targets, blend_attachments.begin(), [](const RenderTargetBlendDesc& render_target) {
                return VkPipelineColorBlendAttachmentState{
                    .blendEnable = render_target.blend_enable,
                    .srcColorBlendFactor = to_vk_blend_factor(render_target.src_blend),
                    .dstColorBlendFactor = to_vk_blend_factor(render_target.dst_blend),
                    .colorBlendOp = to_vk_blend_op(render_target.blend_op),
                    .srcAlphaBlendFactor = to_vk_blend_factor(render_target.src_blend_alpha),
                    .dstAlphaBlendFactor = to_vk_blend_factor(render_target.dst_blend_alpha),
                    .alphaBlendOp = to_vk_blend_op(render_target.blend_op_alpha),
                    .colorWriteMask = to_vk_color_components(render_target.color_write_mask),
                };
            });

            const auto blend = VkPipelineColorBlendStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_NO_OP,
                .attachmentCount = static_cast<uint32_t>(blend_attachments.size()),
                .pAttachments = blend_attachments.data(),
                .blendConstants = {desc.blend.blend_constants[0], desc.blend.blend_constants[1], desc.blend.blend_constants[2], desc.blend.blend_constants[3]},
            };

            // Pipeline dynamic state
            const auto dynamic_states = std::array{
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
            };
            const auto dynamic_state = VkPipelineDynamicStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
                .pDynamicStates = dynamic_states.data(),
            };

            // Dynamic rendering extension
            std::vector<VkFormat> color_attachments(desc.render_target_formats.size());
            std::ranges::transform(desc.render_target_formats, color_attachments.begin(), to_vk_format);

            const auto dynamic_rendering = VkPipelineRenderingCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                .pNext = nullptr,
                .viewMask = 0,
                .colorAttachmentCount = static_cast<uint32_t>(color_attachments.size()),
                .pColorAttachmentFormats = color_attachments.data(),
                .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
                .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
            };

            // Pipeline layout
            VkPipelineLayout pipeline_layout = pipeline_layouts_.lookup(desc.pipeline_layout);
            ORION_EXPECTS(pipeline_layout != VK_NULL_HANDLE);

            const auto info = VkGraphicsPipelineCreateInfo{
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &dynamic_rendering,
                .flags = 0,
                .stageCount = static_cast<uint32_t>(shaders.size()),
                .pStages = shaders.data(),
                .pVertexInputState = &vertex_input,
                .pInputAssemblyState = &input_assembly,
                .pTessellationState = nullptr,
                .pViewportState = &viewport,
                .pRasterizationState = &rasterization,
                .pMultisampleState = &multisample,
                .pDepthStencilState = nullptr,
                .pColorBlendState = &blend,
                .pDynamicState = &dynamic_state,
                .layout = pipeline_layout,
                .renderPass = VK_NULL_HANDLE,
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE,
                .basePipelineIndex = 0,
            };
            vk_assert(vkCreateGraphicsPipelines(device_.get(), VK_NULL_HANDLE, 1, &info, nullptr, &pipeline));
            SPDLOG_TRACE("Created VkPipeline {}", fmt::ptr(pipeline));

            // Cleanup shaders
            // TODO: Do this automatically
            for (VkShaderModule module : shader_modules) {
                vkDestroyShaderModule(device_.get(), module, nullptr);
            }
        }
        return pipelines_.insert(pipeline);
    }

    void VulkanDevice::destroy_api(PipelineLayoutHandle pipeline_layout)
    {
        if (!pipeline_layouts_.remove(pipeline_layout)) {
            SPDLOG_WARN("Attempting to destroy pipeline layout with handle {}, which is not a valid handle", fmt::underlying(pipeline_layout));
        }
    }

    void VulkanDevice::destroy_api(PipelineHandle pipeline)
    {
        if (!pipelines_.remove(pipeline)) {
            SPDLOG_WARN("Attempting to destroy pipeline with handle {}, which is not a valid handle", fmt::underlying(pipeline));
        }
    }

    VkShaderModule VulkanDevice::create_vk_shader_module(std::span<const std::byte> code)
    {
        VkShaderModule shader_module = VK_NULL_HANDLE;
        const auto info = VkShaderModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = code.size_bytes(),
            .pCode = reinterpret_cast<const uint32_t*>(code.data()),
        };
        vk_assert(vkCreateShaderModule(device_.get(), &info, nullptr, &shader_module));
        return shader_module;
    }
} // namespace orion
