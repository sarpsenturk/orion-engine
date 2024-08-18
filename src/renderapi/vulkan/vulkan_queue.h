#pragma once

#include "orion/renderapi/render_queue.h"

#include <volk.h>

namespace orion
{
    class VulkanQueue final : public CommandQueue
    {
    public:
        VulkanQueue(VkQueue queue);

    private:
        VkQueue queue_;
    };
} // namespace orion
