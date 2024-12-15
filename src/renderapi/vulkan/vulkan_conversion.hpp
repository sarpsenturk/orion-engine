#pragma once

#include "orion/renderapi/bind_group.hpp"
#include "orion/renderapi/buffer.hpp"
#include "orion/renderapi/format.hpp"
#include "orion/renderapi/image.hpp"
#include "orion/renderapi/pipeline.hpp"
#include "orion/renderapi/render_command.hpp"

#include <Volk/volk.h>

namespace orion
{
    VkFormat to_vk_format(Format format);
    VkPrimitiveTopology to_vk_primitive_topology(PrimitiveTopology topology);
    VkPolygonMode to_vk_polygon_mode(FillMode mode);
    VkCullModeFlags to_vk_cull_mode(CullMode mode);
    VkBlendFactor to_vk_blend_factor(Blend blend);
    VkBlendOp to_vk_blend_op(BlendOp blend_op);
    VkColorComponentFlags to_vk_color_components(ColorWriteFlags color_write_mask);
    VkBufferUsageFlags to_vk_buffer_usage(BufferUsageFlags usage);
    VkIndexType to_vk_index_type(IndexType index_type);
    VkDescriptorType to_vk_descriptor_type(DescriptorType descriptor_type);
    VkImageType to_vk_image_type(ImageType image_type);
    VkImageUsageFlags to_vk_image_usage(ImageUsageFlags usage);
    VkImageViewType to_vk_image_view_type(ImageViewType image_view_type);
    VkFilter to_vk_filter(Filter filter);
    VkSamplerAddressMode to_vk_address_mode(SamplerAddressMode address_mode);
    VkCompareOp to_vk_compare_op(CompareOp compare_op);
    VkImageLayout to_vk_layout(ImageState image_state);
} // namespace orion
