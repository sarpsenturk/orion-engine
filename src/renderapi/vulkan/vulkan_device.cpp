#include "vulkan_device.hpp"

#include "vulkan_command.hpp"
#include "vulkan_conversion.hpp"
#include "vulkan_error.hpp"
#include "vulkan_platform.hpp"
#include "vulkan_queue.hpp"
#include "vulkan_shader.hpp"
#include "vulkan_swapchain.hpp"

#include "orion/assertion.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <bit>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace orion
{
    namespace
    {
        VmaAllocator create_vma_allocator(VkDevice device, VkPhysicalDevice physical_device, VkInstance instance)
        {
            // Create VMA allocator
            VmaAllocator vma_allocator = VK_NULL_HANDLE;
            {
                const auto vulkan_functions = VmaVulkanFunctions{
                    .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                    .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
                    .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
                    .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
                    .vkAllocateMemory = vkAllocateMemory,
                    .vkFreeMemory = vkFreeMemory,
                    .vkMapMemory = vkMapMemory,
                    .vkUnmapMemory = vkUnmapMemory,
                    .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
                    .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
                    .vkBindBufferMemory = vkBindBufferMemory,
                    .vkBindImageMemory = vkBindImageMemory,
                    .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
                    .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
                    .vkCreateBuffer = vkCreateBuffer,
                    .vkDestroyBuffer = vkDestroyBuffer,
                    .vkCreateImage = vkCreateImage,
                    .vkDestroyImage = vkDestroyImage,
                    .vkCmdCopyBuffer = vkCmdCopyBuffer,
                    .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2,
                    .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2,
                    .vkBindBufferMemory2KHR = vkBindBufferMemory2,
                    .vkBindImageMemory2KHR = vkBindImageMemory2,
                    .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2,
                    .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
                    .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements,
                };
                const auto info = VmaAllocatorCreateInfo{
                    .flags = {},
                    .physicalDevice = physical_device,
                    .device = device,
                    .pVulkanFunctions = &vulkan_functions,
                    .instance = instance,
                    .vulkanApiVersion = VK_API_VERSION_1_2,
                };
                vk_assert(vmaCreateAllocator(&info, &vma_allocator));
                SPDLOG_TRACE("Created VMA allocator {}", fmt::ptr(vma_allocator));
            }
            return vma_allocator;
        }
    } // namespace

    VulkanDevice::VulkanDevice(VkDevice device, VkInstance instance, VkPhysicalDevice physical_device, VkQueue queue, std::uint32_t queue_family_index)
        : device_(device)
        , instance_(instance)
        , physical_device_(physical_device)
        , queue_(queue)
        , queue_family_index_(queue_family_index)
        , vma_allocator_(create_vma_allocator(device, physical_device, instance))
        , context_(device_.get(), vma_allocator_.get())
    {
    }

    VulkanDevice::~VulkanDevice()
    {
        vk_assert(vkDeviceWaitIdle(device_.get()));
    }

    std::unique_ptr<CommandQueue> VulkanDevice::create_command_queue_api()
    {
        return std::make_unique<VulkanQueue>(queue_, &context_);
    }

    std::unique_ptr<Swapchain> VulkanDevice::create_swapchain_api(const SwapchainDesc& desc)
    {
        ORION_EXPECTS(desc.width != 0);
        ORION_EXPECTS(desc.height != 0);

        auto* vulkan_queue = dynamic_cast<VulkanQueue*>(desc.queue);
        ORION_EXPECTS(vulkan_queue != nullptr);

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
        return std::make_unique<VulkanSwapchain>(device_.get(), instance_, physical_device_, surface, swapchain, info, vulkan_queue, &context_);
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
        return std::make_unique<VulkanCommandList>(device_.get(), command_buffer, queue_family_index_, &context_);
    }

    BindGroupLayoutHandle VulkanDevice::create_bind_group_layout_api(const BindGroupLayoutDesc& desc)
    {
        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings(desc.bindings.size());
            std::ranges::transform(desc.bindings, bindings.begin(), [index = 0u](const DescriptorSetBindingDesc& binding) mutable {
                return VkDescriptorSetLayoutBinding{
                    .binding = index++,
                    .descriptorType = to_vk_descriptor_type(binding.type),
                    .descriptorCount = binding.size,
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr,
                };
            });
            const auto info = VkDescriptorSetLayoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .bindingCount = static_cast<std::uint32_t>(bindings.size()),
                .pBindings = bindings.data(),
            };
            vk_assert(vkCreateDescriptorSetLayout(device_.get(), &info, nullptr, &descriptor_set_layout));
            SPDLOG_TRACE("Created VkDescriptorSetLayout {}", fmt::ptr(descriptor_set_layout));
        }
        VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
        {
            std::vector<VkDescriptorPoolSize> pool_sizes(desc.bindings.size());
            std::ranges::transform(desc.bindings, pool_sizes.begin(), [](const DescriptorSetBindingDesc& binding) {
                return VkDescriptorPoolSize{
                    .type = to_vk_descriptor_type(binding.type),
                    .descriptorCount = binding.size,
                };
            });
            const auto info = VkDescriptorPoolCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .maxSets = 1, // TODO: Allow descriptor set layout reuse
                .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
                .pPoolSizes = pool_sizes.data(),
            };
            vk_assert(vkCreateDescriptorPool(device_.get(), &info, nullptr, &descriptor_pool));
            SPDLOG_TRACE("Created VkDescriptorPool {}", fmt::ptr(descriptor_pool));
        }
        return context_.insert(VulkanDescriptorSetLayout(descriptor_set_layout, descriptor_pool));
    }

    PipelineLayoutHandle VulkanDevice::create_pipeline_layout_api([[maybe_unused]] const PipelineLayoutDesc& desc)
    {
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        {
            std::vector<VkDescriptorSetLayout> descriptor_set_layouts(desc.bind_group_layouts.size());
            std::ranges::transform(desc.bind_group_layouts, descriptor_set_layouts.begin(), [this](BindGroupLayoutHandle bind_group_layout) {
                VkDescriptorSetLayout vk_descriptor_set_layout = context_.lookup(bind_group_layout).layout;
                ORION_EXPECTS(vk_descriptor_set_layout != VK_NULL_HANDLE);
                return vk_descriptor_set_layout;
            });
            const auto info = VkPipelineLayoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = static_cast<std::uint32_t>(descriptor_set_layouts.size()),
                .pSetLayouts = descriptor_set_layouts.data(),
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr,
            };
            vk_assert(vkCreatePipelineLayout(device_.get(), &info, nullptr, &pipeline_layout));
            SPDLOG_TRACE("Created VkPipelineLayout {}", fmt::ptr(pipeline_layout));
        }
        return context_.insert(pipeline_layout);
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
            std::uint32_t stride = 0;
            std::vector<VkVertexInputAttributeDescription> vertex_attributes(desc.vertex_attributes.size());
            std::ranges::transform(desc.vertex_attributes, vertex_attributes.begin(), [location = 0u, &stride](const VertexAttribute& attribute) mutable {
                const auto offset = stride;
                stride += static_cast<uint16_t>(format_byte_size(attribute.format));
                return VkVertexInputAttributeDescription{
                    .location = location++,
                    .binding = 0,
                    .format = to_vk_format(attribute.format),
                    .offset = offset,
                };
            });

            // Pipeline vertex binding
            std::vector<VkVertexInputBindingDescription> vertex_bindings;
            if (!vertex_attributes.empty()) {
                vertex_bindings.push_back({
                    .binding = 0,
                    .stride = stride,
                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                });
            }

            // Pipeline vertex input
            const auto vertex_input = VkPipelineVertexInputStateCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_bindings.size()),
                .pVertexBindingDescriptions = vertex_bindings.data(),
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
            VkPipelineLayout pipeline_layout = context_.lookup(desc.pipeline_layout);
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
        return context_.insert(pipeline);
    }

    BufferHandle VulkanDevice::create_buffer_api(const BufferDesc& desc)
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        {
            const auto buffer_info = VkBufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .size = desc.size,
                .usage = to_vk_buffer_usage(desc.usage_flags),
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };
            const auto alloc_info = VmaAllocationCreateInfo{
                .flags = desc.cpu_visible ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : VmaAllocationCreateFlags{},
                .usage = VMA_MEMORY_USAGE_AUTO,
            };
            vk_assert(vmaCreateBuffer(vma_allocator_.get(), &buffer_info, &alloc_info, &buffer, &allocation, nullptr));
            SPDLOG_TRACE("Created VkBuffer {} with VmaAllocation {}", fmt::ptr(buffer), fmt::ptr(allocation));
        }
        return context_.insert(VulkanBuffer(buffer, allocation));
    }

    SemaphoreHandle VulkanDevice::create_semaphore_api(const SemaphoreDesc&)
    {
        VkSemaphore semaphore = VK_NULL_HANDLE;
        {
            const auto info = VkSemaphoreCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
            };
            vk_assert(vkCreateSemaphore(device_.get(), &info, nullptr, &semaphore));
            SPDLOG_TRACE("Created VkSemaphore {}", fmt::ptr(semaphore));
        }
        return context_.insert(semaphore);
    }

    FenceHandle VulkanDevice::create_fence_api(const FenceDesc& desc)
    {
        VkFence fence = VK_NULL_HANDLE;
        {
            const auto info = VkFenceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = desc.signaled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags{},
            };
            vk_assert(vkCreateFence(device_.get(), &info, nullptr, &fence));
            SPDLOG_TRACE("Created VkFence {}", fmt::ptr(fence));
        }
        return context_.insert(fence);
    }

    BindGroupHandle VulkanDevice::create_bind_group_api(const BindGroupDesc& desc)
    {
        VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
        auto [layout, pool] = context_.lookup(desc.layout);
        ORION_ENSURES(layout != VK_NULL_HANDLE);
        ORION_ENSURES(pool != VK_NULL_HANDLE);
        {
            const auto info = VkDescriptorSetAllocateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &layout,
            };
            vk_assert(vkAllocateDescriptorSets(device_.get(), &info, &descriptor_set));
            SPDLOG_TRACE("Created VkDescriptorSet {}", fmt::ptr(descriptor_set));
        }

        // Preallocate buffers
        std::vector<VkDescriptorBufferInfo> buffers;
        buffers.reserve(desc.buffers.size());
        std::vector<VkDescriptorImageInfo> images;
        images.reserve(desc.views.size() + desc.samplers.size());
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(buffers.size() + images.size());

        // Binding index
        std::uint32_t binding = 0;

        // Create descriptor updates
        for (const auto& buffer : desc.buffers) {
            VkBuffer vk_buffer = context_.lookup(buffer.buffer).buffer;
            ORION_EXPECTS(vk_buffer != VK_NULL_HANDLE);
            buffers.push_back(VkDescriptorBufferInfo{
                .buffer = vk_buffer,
                .offset = buffer.offset,
                .range = buffer.size,
            });
            writes.push_back(VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_set,
                .dstBinding = binding++,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = to_vk_descriptor_type(buffer.type),
                .pImageInfo = nullptr,
                .pBufferInfo = &buffers.back(),
                .pTexelBufferView = nullptr,
            });
        }
        for (const auto& image : desc.views) {
            VkImageView vk_image_view = context_.lookup(image.image_view);
            ORION_EXPECTS(vk_image_view != VK_NULL_HANDLE);
            images.push_back(VkDescriptorImageInfo{
                .sampler = VK_NULL_HANDLE,
                .imageView = vk_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            });
            writes.push_back(VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_set,
                .dstBinding = binding++,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .pImageInfo = &images.back(),
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr,
            });
        }
        for (const auto& sampler : desc.samplers) {
            VkSampler vk_sampler = context_.lookup(sampler.sampler);
            ORION_EXPECTS(vk_sampler != VK_NULL_HANDLE);
            images.push_back(VkDescriptorImageInfo{
                .sampler = vk_sampler,
                .imageView = VK_NULL_HANDLE,
                .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            });
            writes.push_back(VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_set,
                .dstBinding = binding++,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .pImageInfo = &images.back(),
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr,
            });
        }

        // Update descriptor set
        vkUpdateDescriptorSets(device_.get(), static_cast<std::uint32_t>(writes.size()), writes.data(), 0, nullptr);

        return context_.insert(descriptor_set, pool);
    }

    ImageHandle VulkanDevice::create_image_api(const ImageDesc& desc)
    {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        const auto image_info = VkImageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .imageType = to_vk_image_type(desc.type),
            .format = to_vk_format(desc.format),
            .extent = {.width = desc.width, .height = desc.height, .depth = desc.depth},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = to_vk_image_usage(desc.usage_flags),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        const auto alloc_info = VmaAllocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        vk_assert(vmaCreateImage(vma_allocator_.get(), &image_info, &alloc_info, &image, &allocation, nullptr));
        SPDLOG_TRACE("Created VkImage {} with VmaAllocation {}", fmt::ptr(image), fmt::ptr(allocation));
        return context_.insert(VulkanImage(image, allocation), {.create_info = image_info});
    }

    ImageViewHandle VulkanDevice::create_image_view_api(const ImageViewDesc& desc)
    {
        VkImageView image_view = VK_NULL_HANDLE;
        {
            VkImage vk_image = context_.lookup(desc.image).image;
            VkFormat format = context_.lookup_data(desc.image)->create_info.format;
            const auto info = VkImageViewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .image = vk_image,
                .viewType = to_vk_image_view_type(desc.type),
                .format = format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                // TODO: Make this customizable
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            vk_assert(vkCreateImageView(device_.get(), &info, nullptr, &image_view));
            SPDLOG_TRACE("Created VkImageView {}", fmt::ptr(image_view));
        }
        return context_.insert(image_view);
    }

    SamplerHandle VulkanDevice::create_sampler_api(const SamplerDesc& desc)
    {
        VkSampler sampler = VK_NULL_HANDLE;
        {
            const auto info = VkSamplerCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .magFilter = to_vk_filter(desc.filter),
                .minFilter = to_vk_filter(desc.filter),
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = to_vk_address_mode(desc.address_mode_u),
                .addressModeV = to_vk_address_mode(desc.address_mode_v),
                .addressModeW = to_vk_address_mode(desc.address_mode_w),
                .mipLodBias = desc.mip_lod_bias,
                .anisotropyEnable = VK_FALSE,
                .maxAnisotropy = 1.f,
                .compareEnable = desc.compare_op != CompareOp::None,
                .compareOp = to_vk_compare_op(desc.compare_op),
                .minLod = desc.min_lod,
                .maxLod = desc.max_lod,
                .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
                .unnormalizedCoordinates = VK_FALSE,
            };
            vk_assert(vkCreateSampler(device_.get(), &info, nullptr, &sampler));
            SPDLOG_TRACE("Created VkSampler {}", fmt::ptr(sampler));
        }
        return context_.insert(sampler);
    }

    void VulkanDevice::destroy_api(BindGroupLayoutHandle bind_group_layout)
    {
        if (!context_.remove(bind_group_layout)) {
            SPDLOG_WARN("Attempting to destroy bind group layout with handle {}, which is not a valid handle", fmt::underlying(bind_group_layout));
        }
    }

    void VulkanDevice::destroy_api(PipelineLayoutHandle pipeline_layout)
    {
        if (!context_.remove(pipeline_layout)) {
            SPDLOG_WARN("Attempting to destroy pipeline layout with handle {}, which is not a valid handle", fmt::underlying(pipeline_layout));
        }
    }

    void VulkanDevice::destroy_api(PipelineHandle pipeline)
    {
        if (!context_.remove(pipeline)) {
            SPDLOG_WARN("Attempting to destroy pipeline with handle {}, which is not a valid handle", fmt::underlying(pipeline));
        }
    }

    void VulkanDevice::destroy_api(BufferHandle buffer)
    {
        if (!context_.remove(buffer)) {
            SPDLOG_WARN("Attempting to destroy buffer with handle {}, which is not a valid handle", fmt::underlying(buffer));
        }
    }

    void VulkanDevice::destroy_api(SemaphoreHandle semaphore)
    {
        if (!context_.remove(semaphore)) {
            SPDLOG_WARN("Attempting to destroy semaphore with handle {}, which is not a valid handle", fmt::underlying(semaphore));
        }
    }

    void VulkanDevice::destroy_api(FenceHandle fence)
    {
        if (!context_.remove(fence)) {
            SPDLOG_WARN("Attempting to destroy fence with handle {}, which is not a valid handle", fmt::underlying(fence));
        }
    }

    void VulkanDevice::destroy_api(BindGroupHandle descriptor_set)
    {
        if (!context_.remove(descriptor_set)) {
            SPDLOG_WARN("Attempting to destroy bind group with handle {}, which is not a valid handle", fmt::underlying(descriptor_set));
        }
    }

    void VulkanDevice::destroy_api(ImageHandle image)
    {
        if (!context_.remove(image)) {
            SPDLOG_WARN("Attempting to destroy image with handle {}, which is invalid", fmt::underlying(image));
        }
    }

    void VulkanDevice::destroy_api(ImageViewHandle image_view)
    {
        if (!context_.remove(image_view)) {
            SPDLOG_WARN("Attempting to destroy image view with handle {}, which is not a valid handle", fmt::underlying(image_view));
        }
    }

    void VulkanDevice::destroy_api(SamplerHandle sampler)
    {
        if (!context_.remove(sampler)) {
            SPDLOG_WARN("Attempting to destroy sampler with handle {}, which is not a valid handle", fmt::underlying(sampler));
        }
    }

    void* VulkanDevice::map_api(BufferHandle buffer)
    {
        if (VulkanBuffer vk_buffer = context_.lookup(buffer)) {
            void* ptr;
            vk_assert(vmaMapMemory(vma_allocator_.get(), vk_buffer.allocation, &ptr));
            return ptr;
        } else {
            SPDLOG_ERROR("Failed to map buffer {}: failed to find buffer", fmt::underlying(buffer));
            return nullptr;
        }
    }

    void VulkanDevice::unmap_api(BufferHandle buffer)
    {
        if (VulkanBuffer vk_buffer = context_.lookup(buffer)) {
            vmaUnmapMemory(vma_allocator_.get(), vk_buffer.allocation);
        } else {
            SPDLOG_ERROR("Failed to unmap buffer {}: failed to find buffer", fmt::underlying(buffer));
        }
    }

    void VulkanDevice::wait_for_fence_api(FenceHandle fence)
    {
        VkFence vk_fence = context_.lookup(fence);
        ORION_EXPECTS(vk_fence != VK_NULL_HANDLE);
        vk_assert(vkWaitForFences(device_.get(), 1, &vk_fence, VK_TRUE, UINT64_MAX));
        vk_assert(vkResetFences(device_.get(), 1, &vk_fence));
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
