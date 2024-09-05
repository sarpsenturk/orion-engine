#pragma once

#include "orion/renderapi/handle.h"

#include <Volk/volk.h>

#include <array>
#include <cstdint>
#include <memory>

namespace orion
{
    template<typename VkHandle, typename OrionHandle, typename Deleter>
    class VulkanHandleTable
    {
    public:
        explicit VulkanHandleTable(VkDevice device, Deleter deleter = {})
            : device_(device)
            , deleter_(std::move(deleter))
        {
        }

        ~VulkanHandleTable()
        {
            for (const auto [handle, _] : table_) {
                if (handle != VK_NULL_HANDLE) {
                    destroy(handle);
                }
            }
        }

        OrionHandle insert(VkHandle handle)
        {
            const auto slot = find_empty_slot();
            if (slot == UINT16_MAX) {
                return OrionHandle::Invalid;
            }
            auto& entry = table_[slot];
            entry.handle = handle;
            return static_cast<OrionHandle>(entry.gen << 16 | slot);
        }

        VkHandle lookup(OrionHandle handle)
        {
            const auto [slot, gen] = to_vulkan_handle(handle);
            if (table_[slot].gen == gen) {
                return table_[slot].handle;
            } else {
                return VK_NULL_HANDLE;
            }
        }

        bool remove(OrionHandle handle)
        {
            const auto [slot, gen] = to_vulkan_handle(handle);
            if (auto& entry = table_[slot]; entry.gen == gen) {
                destroy(entry.handle);
                entry.handle = VK_NULL_HANDLE;
                entry.gen += 1;
                return true;
            } else {
                return false;
            }
        }

    private:
        struct Entry {
            VkHandle handle = VK_NULL_HANDLE;
            std::uint16_t gen = 0;

            [[nodiscard]] bool is_empty() const noexcept { return handle == VK_NULL_HANDLE; }
        };

        void destroy(VkHandle vk_handle)
        {
            deleter_(device_, vk_handle);
        }

        std::uint16_t find_empty_slot()
        {
            for (std::uint16_t slot = 0; slot < table_.size(); ++slot) {
                if (table_[slot].is_empty())
                    return slot;
            }
            return UINT16_MAX;
        }

        struct VulkanHandleImpl {
            std::uint16_t slot;
            std::uint16_t gen;
        };

        VulkanHandleImpl to_vulkan_handle(auto handle)
        {
            const auto device_handle = static_cast<render_device_handle_t>(handle);
            return {.slot = device_handle & 0xffff, .gen = device_handle >> 16 & 0xffff};
        }

        std::array<Entry, 0xffff> table_;
        VkDevice device_;
        Deleter deleter_;
    };

    struct VulkanDeviceDeleter {
        using pointer = VkDevice;
        void operator()(VkDevice device) const;
    };
    using UniqueVulkanDevice = std::unique_ptr<VkDevice, VulkanDeviceDeleter>;

    struct VulkanPipelineLayoutDeleter {
        void operator()(VkDevice device, VkPipelineLayout pipelineLayout) const;
    };

    struct VulkanPipelineDeleter {
        void operator()(VkDevice device, VkPipeline pipeline) const;
    };

    using VulkanPipelineLayoutTable = VulkanHandleTable<VkPipelineLayout, PipelineLayoutHandle, VulkanPipelineLayoutDeleter>;
    using VulkanPipelineTable = VulkanHandleTable<VkPipeline, PipelineHandle, VulkanPipelineDeleter>;
} // namespace orion
