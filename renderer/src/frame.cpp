#include "orion-renderer/frame.h"

#include "orion-renderapi/render_device.h"

namespace orion
{
    PerFrame<UniqueDescriptor> create_descriptor_per_frame(RenderDevice* device, DescriptorLayoutHandle layout, DescriptorPoolHandle pool)
    {
        return generate_per_frame([&](frame_index_t) { return device->make_unique<DescriptorHandle_tag>(layout, pool); });
    }
} // namespace orion
