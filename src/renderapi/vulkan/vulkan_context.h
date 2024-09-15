#pragma once

#include "orion/renderapi/handle.h"

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

#include <array>

namespace orion
{
    struct VulkanBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        explicit(false) operator bool() const noexcept { return buffer != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE; }
    };

    class VulkanContext
    {
    public:
        VulkanContext(VkDevice device, VmaAllocator allocator);
        ~VulkanContext();

        PipelineLayoutHandle insert(VkPipelineLayout pipeline_layout);
        PipelineHandle insert(VkPipeline pipeline);
        BufferHandle insert(VkBuffer buffer, VmaAllocation allocation);
        ImageHandle insert(VkImage image);
        RenderTargetHandle insert(VkImageView image_view);
        SemaphoreHandle insert(VkSemaphore semaphore);
        FenceHandle insert(VkFence fence);

        VkPipelineLayout lookup(PipelineLayoutHandle pipeline_layout) const;
        VkPipeline lookup(PipelineHandle pipeline) const;
        VulkanBuffer lookup(BufferHandle buffer) const;
        VkImage lookup(ImageHandle image) const;
        VkImageView lookup(RenderTargetHandle render_target) const;
        VkSemaphore lookup(SemaphoreHandle semaphore) const;
        VkFence lookup(FenceHandle fence) const;

        bool remove(PipelineLayoutHandle pipeline_layout);
        bool remove(PipelineHandle pipeline);
        bool remove(BufferHandle buffer);
        bool remove(ImageHandle image);
        bool remove(RenderTargetHandle render_target);
        bool remove(SemaphoreHandle semaphore);
        bool remove(FenceHandle fence);

    private:
        template<typename T>
        struct TableEntry {
            T value = {};
            std::uint16_t gen = 0;
        };

        static_assert(sizeof(TableEntry<VkPipelineLayout>) == 16);

        template<typename T>
        using ResourceTable = std::array<TableEntry<T>, 0xffff>;

        template<typename T>
        static bool is_empty(const TableEntry<T>& entry)
        {
            return entry.value == VK_NULL_HANDLE;
        }

        template<>
        bool is_empty(const TableEntry<VulkanBuffer>& entry)
        {
            return entry.value.buffer == VK_NULL_HANDLE;
        }

        template<typename T>
        static std::uint16_t find_empty_slot(const ResourceTable<T>& table)
        {
            for (std::uint16_t i = 0; i < table.size(); ++i) {
                if (is_empty(table[i])) {
                    return i;
                }
            }
            return UINT16_MAX;
        }

        template<typename T>
        static render_device_handle_t get_device_handle(const ResourceTable<T>& table, std::uint16_t index)
        {
            return index | table[index].gen << 16;
        }

        template<typename T>
        static render_device_handle_t insert(ResourceTable<T>& table, T value)
        {
            const auto slot = find_empty_slot(table);
            if (slot == UINT16_MAX) {
                return invalid_device_handle;
            }
            table[slot].value = value;
            return get_device_handle(table, slot);
        }

        struct VulkanHandleImpl {
            std::uint16_t index = 0;
            std::uint16_t gen = 0;
        };

        static VulkanHandleImpl to_vulkan_handle(render_device_handle_t handle)
        {
            return {.index = static_cast<std::uint16_t>(handle & 0xffff), .gen = static_cast<std::uint16_t>(handle >> 16)};
        }

        template<typename T>
        static T lookup(const ResourceTable<T>& table, render_device_handle_t handle)
        {
            const auto [index, gen] = to_vulkan_handle(handle);
            if (index >= table.size()) {
                return {};
            }
            const auto entry = table[index];
            if (entry.gen != gen) {
                return {};
            }
            return entry.value;
        }

        template<typename T>
        static bool remove(ResourceTable<T>& table, render_device_handle_t handle, auto deleter)
        {
            const auto [index, gen] = to_vulkan_handle(handle);
            if (index >= table.size()) {
                return false;
            }
            auto& entry = table[index];
            if (is_empty(entry) || entry.gen != gen) {
                return false;
            }
            deleter(entry.value);
            entry.value = {};
            entry.gen += 1;
            return true;
        }

        VkDevice device_;
        VmaAllocator allocator_;

        ResourceTable<VkPipelineLayout> pipeline_layouts_;
        ResourceTable<VkPipeline> pipelines_;
        ResourceTable<VulkanBuffer> buffers_;
        ResourceTable<VkImage> images_;
        ResourceTable<VkImageView> image_views_;
        ResourceTable<VkSemaphore> semaphores_;
        ResourceTable<VkFence> fences_;
    };
} // namespace orion
