#pragma once

#include "vulkan_headers.h"

#include "orion-renderapi/defs.h"

#include "orion-math/vector/vector2.h"

namespace orion::vulkan
{
    PhysicalDeviceType to_orion_type(VkPhysicalDeviceType physical_device_type);

    VkFormat to_vulkan_type(Format format);
    VkAttachmentLoadOp to_vulkan_type(AttachmentLoadOp load_op);
    VkAttachmentStoreOp to_vulkan_type(AttachmentStoreOp store_op);
    VkImageLayout to_vulkan_type(ImageLayout image_layout);
    VkShaderStageFlags to_vulkan_type(ShaderStageFlags shader_stages);
    VkPrimitiveTopology to_vulkan_type(PrimitiveTopology topology);
    VkPolygonMode to_vulkan_type(FillMode fill_mode);
    VkCullModeFlags to_vulkan_type(CullMode cull_mode);
    VkFrontFace to_vulkan_type(FrontFace front_face);
    VkBufferUsageFlags to_vulkan_type(GPUBufferUsageFlags buffer_usage);
    VkViewport to_vulkan_type(const Viewport& viewport);
    VkDescriptorPoolCreateFlags to_vulkan_type(DescriptorPoolFlags flags);
    VkDescriptorType to_vulkan_type(DescriptorType descriptor_type);
    VkImageType to_vulkan_type(ImageType image_type);
    VkImageTiling to_vulkan_type(ImageTiling image_tiling);
    VkImageUsageFlags to_vulkan_type(ImageUsageFlags image_usage_flags);
    VkImageViewType to_vulkan_type(ImageViewType image_view_type);
    VkIndexType to_vulkan_type(IndexType index_type);
    VkFilter to_vulkan_type(Filter filter);
    VkSamplerAddressMode to_vulkan_type(AddressMode address_mode);
    VkCompareOp to_vulkan_type(CompareFunc compare_func);
    VkBlendFactor to_vulkan_type(BlendFactor blend_factor);
    VkBlendOp to_vulkan_type(BlendOp blend_op);
    VkColorComponentFlags to_vulkan_type(ColorComponentFlags color_component_flags);
    VkLogicOp to_vulkan_type(LogicOp logic_op);
    VkPipelineBindPoint to_vulkan_type(PipelineBindPoint bind_point);
    VkViewport to_vulkan_viewport(const Viewport& viewport);
    VkRect2D to_vulkan_scissor(const Scissor& scissor);
    VkExtent2D to_vulkan_extent(const Vector2_u& vec2);
    VkExtent3D to_vulkan_extent(const Vector3_u& vec3);
    VkOffset2D to_vulkan_offset(const Vector2_i& vec2);
    VkRect2D to_vulkan_rect(const Rect2D& rect2d);
    VkClearColorValue to_vulkan_clear_color(const Vector4_f& color);
} // namespace orion::vulkan
