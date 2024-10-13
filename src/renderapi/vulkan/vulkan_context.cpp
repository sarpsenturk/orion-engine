#include "vulkan_context.h"

namespace orion
{
    namespace
    {
        template<typename T>
        bool is_empty(const VulkanContext::TableEntry<T>& entry)
        {
            return entry.value == VK_NULL_HANDLE;
        }

        template<>
        bool is_empty(const VulkanContext::TableEntry<VulkanBuffer>& entry)
        {
            return entry.value.buffer == VK_NULL_HANDLE;
        }

        template<>
        bool is_empty(const VulkanContext::TableEntry<VulkanDescriptorSet>& entry)
        {
            return entry.value.set == VK_NULL_HANDLE;
        }

        template<>
        bool is_empty(const VulkanContext::TableEntry<VulkanImage>& entry)
        {
            return entry.value.image == VK_NULL_HANDLE;
        }

        template<typename T>
        std::uint16_t find_empty_slot(const VulkanContext::ResourceTable<T>& table)
        {
            for (std::uint16_t i = 0; i < table.size(); ++i) {
                if (is_empty(table[i])) {
                    return i;
                }
            }
            return UINT16_MAX;
        }

        template<typename T>
        render_device_handle_t get_device_handle(const VulkanContext::ResourceTable<T>& table, std::uint16_t index)
        {
            return index | table[index].gen << 16;
        }

        template<typename T>
        render_device_handle_t insert_t(VulkanContext::ResourceTable<T>& table, T value)
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

        VulkanHandleImpl to_vulkan_handle(render_device_handle_t handle)
        {
            return {.index = static_cast<std::uint16_t>(handle & 0xffff), .gen = static_cast<std::uint16_t>(handle >> 16)};
        }

        template<typename T>
        T lookup_t(const VulkanContext::ResourceTable<T>& table, render_device_handle_t handle)
        {
            const auto [index, gen] = to_vulkan_handle(handle);
            if (index >= table.size()) {
                return {};
            }
            const auto& entry = table[index];
            if (entry.gen != gen) {
                return {};
            }
            return entry.value;
        }

        template<typename T>
        bool remove_t(VulkanContext::ResourceTable<T>& table, render_device_handle_t handle, auto deleter)
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
    } // namespace

    VulkanContext::VulkanContext(VkDevice device, VmaAllocator allocator)
        : device_(device)
        , allocator_(allocator)
    {
    }

    VulkanContext::~VulkanContext()
    {
        // Don't need to call vkFreeDescriptorSets as vkDestroyDescriptorPool will handle it
        for (const auto [descriptor_pool, _] : descriptor_pools_) {
            vkDestroyDescriptorPool(device_, descriptor_pool, nullptr);
        }

        for (const auto [fence, _] : fences_) {
            vkDestroyFence(device_, fence, nullptr);
        }

        for (const auto [semaphore, _] : semaphores_) {
            vkDestroySemaphore(device_, semaphore, nullptr);
        }

        for (const auto [image_view, _] : image_views_) {
            vkDestroyImageView(device_, image_view, nullptr);
        }

        // TODO: Destroy user created images
        for (const auto [image, _] : images_) {
            if (image.is_user_image()) {
                vmaDestroyImage(allocator_, image.image, image.allocation);
            }
        }

        for (const auto [buffer, _] : buffers_) {
            vmaDestroyBuffer(allocator_, buffer.buffer, buffer.allocation);
        }

        for (const auto [pipeline, _] : pipelines_) {
            vkDestroyPipeline(device_, pipeline, nullptr);
        }

        for (const auto [pipeline_layout, _] : pipeline_layouts_) {
            vkDestroyPipelineLayout(device_, pipeline_layout, nullptr);
        }

        for (const auto [descriptor_set_layout, _] : descriptor_set_layouts_) {
            vkDestroyDescriptorSetLayout(device_, descriptor_set_layout, nullptr);
        }

        vmaDestroyAllocator(allocator_);

        vkDestroyDevice(device_, nullptr);
    }

    DescriptorSetLayoutHandle VulkanContext::insert(VkDescriptorSetLayout descriptor_set_layout)
    {
        return DescriptorSetLayoutHandle{insert_t(descriptor_set_layouts_, descriptor_set_layout)};
    }

    PipelineLayoutHandle VulkanContext::insert(VkPipelineLayout pipeline_layout)
    {
        return PipelineLayoutHandle{insert_t(pipeline_layouts_, pipeline_layout)};
    }

    PipelineHandle VulkanContext::insert(VkPipeline pipeline)
    {
        return PipelineHandle{insert_t(pipelines_, pipeline)};
    }

    BufferHandle VulkanContext::insert(VkBuffer buffer, VmaAllocation allocation)
    {
        return BufferHandle{insert_t(buffers_, VulkanBuffer{.buffer = buffer, .allocation = allocation})};
    }

    ImageHandle VulkanContext::insert(VkImage image, VmaAllocation allocation)
    {
        return ImageHandle{insert_t(images_, VulkanImage{.image = image, .allocation = allocation})};
    }

    ImageViewHandle VulkanContext::insert(VkImageView image_view)
    {
        return ImageViewHandle{insert_t(image_views_, image_view)};
    }

    SemaphoreHandle VulkanContext::insert(VkSemaphore semaphore)
    {
        return SemaphoreHandle{insert_t(semaphores_, semaphore)};
    }

    FenceHandle VulkanContext::insert(VkFence fence)
    {
        return FenceHandle{insert_t(fences_, fence)};
    }

    DescriptorPoolHandle VulkanContext::insert(VkDescriptorPool descriptor_pool)
    {
        return DescriptorPoolHandle{insert_t(descriptor_pools_, descriptor_pool)};
    }

    DescriptorSetHandle VulkanContext::insert(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool)
    {
        return DescriptorSetHandle{insert_t(descriptor_sets_, VulkanDescriptorSet{descriptor_set, descriptor_pool})};
    }

    VkDescriptorSetLayout VulkanContext::lookup(DescriptorSetLayoutHandle descriptor_set_layout) const
    {
        return lookup_t(descriptor_set_layouts_, static_cast<render_device_handle_t>(descriptor_set_layout));
    }

    VkPipelineLayout VulkanContext::lookup(PipelineLayoutHandle pipeline_layout) const
    {
        return lookup_t(pipeline_layouts_, static_cast<render_device_handle_t>(pipeline_layout));
    }

    VkPipeline VulkanContext::lookup(PipelineHandle pipeline) const
    {
        return lookup_t(pipelines_, static_cast<render_device_handle_t>(pipeline));
    }

    VulkanBuffer VulkanContext::lookup(BufferHandle buffer) const
    {
        return lookup_t(buffers_, static_cast<render_device_handle_t>(buffer));
    }

    VkImage VulkanContext::lookup(ImageHandle image) const
    {
        return lookup_t(images_, static_cast<render_device_handle_t>(image)).image;
    }

    VkImageView VulkanContext::lookup(ImageViewHandle image_view) const
    {
        return lookup_t(image_views_, static_cast<render_device_handle_t>(image_view));
    }

    VkSemaphore VulkanContext::lookup(SemaphoreHandle semaphore) const
    {
        return lookup_t(semaphores_, static_cast<render_device_handle_t>(semaphore));
    }

    VkFence VulkanContext::lookup(FenceHandle fence) const
    {
        return lookup_t(fences_, static_cast<render_device_handle_t>(fence));
    }

    VkDescriptorPool VulkanContext::lookup(DescriptorPoolHandle descriptor_pool) const
    {
        return lookup_t(descriptor_pools_, static_cast<render_device_handle_t>(descriptor_pool));
    }

    VkDescriptorSet VulkanContext::lookup(DescriptorSetHandle descriptor_set) const
    {
        return lookup_t(descriptor_sets_, static_cast<render_device_handle_t>(descriptor_set)).set;
    }

    bool VulkanContext::remove(DescriptorSetLayoutHandle descriptor_set_layout)
    {
        auto deleter = [this](VkDescriptorSetLayout descriptor_set_layout) {
            vkDestroyDescriptorSetLayout(device_, descriptor_set_layout, nullptr);
        };
        return remove_t(descriptor_set_layouts_, static_cast<render_device_handle_t>(descriptor_set_layout), deleter);
    }

    bool VulkanContext::remove(PipelineLayoutHandle pipeline_layout)
    {
        auto deleter = [this](VkPipelineLayout pipeline_layout) {
            vkDestroyPipelineLayout(device_, pipeline_layout, nullptr);
        };
        return remove_t(pipeline_layouts_, static_cast<render_device_handle_t>(pipeline_layout), deleter);
    }

    bool VulkanContext::remove(PipelineHandle pipeline)
    {
        auto deleter = [this](VkPipeline pipeline) {
            vkDestroyPipeline(device_, pipeline, nullptr);
        };
        return remove_t(pipelines_, static_cast<render_device_handle_t>(pipeline), deleter);
    }

    bool VulkanContext::remove(BufferHandle buffer)
    {
        auto deleter = [this](VulkanBuffer buffer) {
            vmaDestroyBuffer(allocator_, buffer.buffer, buffer.allocation);
        };
        return remove_t(buffers_, static_cast<render_device_handle_t>(buffer), deleter);
    }

    bool VulkanContext::remove(ImageHandle image)
    {
        return remove_t(images_, static_cast<render_device_handle_t>(image), [this](VulkanImage vk_image) {
            if (vk_image.is_user_image()) {
                vmaDestroyImage(allocator_, vk_image.image, vk_image.allocation);
            }
        });
    }

    bool VulkanContext::remove(ImageViewHandle render_target)
    {
        auto deleter = [this](VkImageView image_view) {
            vkDestroyImageView(device_, image_view, nullptr);
        };
        return remove_t(image_views_, static_cast<render_device_handle_t>(render_target), deleter);
    }

    bool VulkanContext::remove(SemaphoreHandle semaphore)
    {
        auto deleter = [this](VkSemaphore semaphore) {
            vkDestroySemaphore(device_, semaphore, nullptr);
        };
        return remove_t(semaphores_, static_cast<render_device_handle_t>(semaphore), deleter);
    }

    bool VulkanContext::remove(FenceHandle fence)
    {
        auto deleter = [this](VkFence fence) {
            vkDestroyFence(device_, fence, nullptr);
        };
        return remove_t(fences_, static_cast<render_device_handle_t>(fence), deleter);
    }

    bool VulkanContext::remove(DescriptorPoolHandle descriptor_pool)
    {
        auto deleter = [this](VkDescriptorPool descriptor_pool) {
            vkDestroyDescriptorPool(device_, descriptor_pool, nullptr);
        };
        return remove_t(descriptor_pools_, static_cast<render_device_handle_t>(descriptor_pool), deleter);
    }

    bool VulkanContext::remove(DescriptorSetHandle descriptor_set)
    {
        auto deleter = [this](const VulkanDescriptorSet& descriptor_set) {
            vkFreeDescriptorSets(device_, descriptor_set.pool, 1, &descriptor_set.set);
        };
        return remove_t(descriptor_sets_, static_cast<render_device_handle_t>(descriptor_set), deleter);
    }
} // namespace orion
