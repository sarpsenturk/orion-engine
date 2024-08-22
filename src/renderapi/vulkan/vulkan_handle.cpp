#include "vulkan_handle.h"

#include "orion/assertion.h"

#include <algorithm>

namespace orion
{
    namespace
    {
        template<typename Entry>
        std::uint16_t find_empty_slot(const std::array<Entry, 0xffff>& table)
        {
            for (std::uint16_t slot = 0; slot < table.size(); ++slot) {
                if (table[slot].is_empty())
                    return slot;
            }
            return UINT16_MAX;
        }

        struct VulkanHandle {
            std::uint16_t slot;
            std::uint16_t gen;
        };

        VulkanHandle to_vulkan_handle(auto handle)
        {
            const auto device_handle = static_cast<render_device_handle_t>(handle);
            return {.slot = device_handle & 0xffff, .gen = device_handle >> 16 & 0xffff};
        }
    } // namespace

    VulkanHandleTable::VulkanHandleTable(VkDevice device)
        : device_(device)
    {
    }

    VulkanHandleTable::~VulkanHandleTable()
    {
        for (auto [pipeline, _] : pipelines_) {
            if (pipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(device_, pipeline, nullptr);
            }
        }
    }

    GraphicsPipelineHandle VulkanHandleTable::insert(VkPipeline pipeline)
    {
        const auto slot = find_empty_slot(pipelines_);
        if (slot == UINT16_MAX) {
            return GraphicsPipelineHandle{invalid_device_handle};
        }

        auto& entry = pipelines_[slot];
        entry.handle = pipeline;
        return static_cast<GraphicsPipelineHandle>(entry.gen << 16 | slot);
    }

    VkPipeline VulkanHandleTable::get(GraphicsPipelineHandle handle) const
    {
        const auto [slot, gen] = to_vulkan_handle(handle);
        ORION_ASSERT(slot < 0xffff);
        if (pipelines_[slot].gen == gen) {
            return pipelines_[slot].handle;
        } else {
            return VK_NULL_HANDLE;
        }
    }

    bool VulkanHandleTable::remove(GraphicsPipelineHandle handle)
    {
        const auto [slot, gen] = to_vulkan_handle(handle);
        ORION_ASSERT(slot < pipelines_.size());
        if (auto& entry = pipelines_[slot]; entry.gen == gen) {
            vkDestroyPipeline(device_, entry.handle, nullptr);
            entry.handle = VK_NULL_HANDLE;
            entry.gen += 1;
            return true;
        } else {
            return false;
        }
    }
} // namespace orion
