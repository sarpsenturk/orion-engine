#pragma once

#include "orion/renderapi/format.h"
#include "orion/renderapi/pipeline.h"

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
} // namespace orion
