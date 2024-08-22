#pragma once

#include "orion/renderapi/handle.h"

#include <Volk/volk.h>

#include <array>
#include <cstdint>

namespace orion
{
    class VulkanHandleTable
    {
        static_assert(sizeof(render_device_handle_t) >= 4, "VulkanHandleTable expects render_device_handle_t to be at least 32 bits");
        // VulkanHandleTable handles:
        //  bits 0-15: index
        //  bits 16-31: generation

    public:
        explicit VulkanHandleTable(VkDevice device);
        ~VulkanHandleTable();

        GraphicsPipelineHandle insert(VkPipeline pipeline);

        VkPipeline get(GraphicsPipelineHandle handle) const;

        bool remove(GraphicsPipelineHandle handle);

    private:
        template<typename T>
        struct Entry {
            T handle = VK_NULL_HANDLE;
            std::uint16_t gen = 0;

            [[nodiscard]] bool is_empty() const noexcept { return handle == VK_NULL_HANDLE; }
        };

        VkDevice device_;

        std::array<Entry<VkPipeline>, 0xffff> pipelines_;
    };
} // namespace orion
