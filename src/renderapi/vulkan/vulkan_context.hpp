#pragma once

#include "orion/renderapi/handle.hpp"

#include "vulkan_buffer.hpp"
#include "vulkan_descriptor.hpp"
#include "vulkan_image.hpp"
#include "vulkan_raii.hpp"

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

#include <array>
#include <stdexcept>

namespace orion
{
    template<typename T>
    class VulkanPool
    {
    public:
        render_device_handle_t insert(T vk_handle)
        {
            const auto index = find_empty_index();
            if (index == UINT16_MAX) {
                throw std::runtime_error("exceeded maximum number of resources (65535)");
            }
            vk_handles_[index] = std::move(vk_handle);
            const auto gen = gens_[index];
            const auto handle = static_cast<render_device_handle_t>(gen << 16 | index);
            return handle;
        }

        auto lookup(render_device_handle_t handle) const -> decltype(std::declval<T>().get())
        {
            const auto index = static_cast<std::uint16_t>(handle & 0xff);
            const auto gen = static_cast<std::uint16_t>(handle >> 16);
            if (gen != gens_[index]) {
                return nullptr;
            } else {
                return vk_handles_[index].get();
            }
        }

        bool remove(render_device_handle_t handle)
        {
            const auto index = static_cast<std::uint16_t>(handle & 0xff);
            const auto gen = static_cast<std::uint16_t>(handle >> 16);
            if (gen != gens_[index]) {
                return false;
            } else {
                vk_handles_[index] = nullptr;
                gens_[index] += 1;
                return true;
            }
        }

    private:
        // TODO: We can use a free list here speed things up
        [[nodiscard]] std::uint16_t find_empty_index() const
        {
            for (std::uint16_t i = 0; i < vk_handles_.size(); ++i) {
                if (vk_handles_[i] == VK_NULL_HANDLE) {
                    return i;
                }
            }
            return UINT16_MAX;
        }

        std::array<T, 0xffff> vk_handles_ = {};
        std::array<std::uint16_t, 0xffff> gens_ = {};
    };

    class VulkanContext
    {
    public:
        VulkanContext(VkDevice device, VmaAllocator allocator);

        BindGroupLayoutHandle insert(VulkanDescriptorSetLayout descriptor_set_layout);
        PipelineLayoutHandle insert(VkPipelineLayout pipeline_layout);
        PipelineHandle insert(VkPipeline pipeline);
        BufferHandle insert(VulkanBuffer buffer);
        ImageHandle insert(VulkanImage image);
        ImageViewHandle insert(VkImageView image_view);
        SemaphoreHandle insert(VkSemaphore semaphore);
        FenceHandle insert(VkFence fence);
        BindGroupHandle insert(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool);
        SamplerHandle insert(VkSampler sampler);

        [[nodiscard]] VulkanDescriptorSetLayout lookup(BindGroupLayoutHandle bind_group_layout) const;
        [[nodiscard]] VkPipelineLayout lookup(PipelineLayoutHandle pipeline_layout) const;
        [[nodiscard]] VkPipeline lookup(PipelineHandle pipeline) const;
        [[nodiscard]] VulkanBuffer lookup(BufferHandle buffer) const;
        [[nodiscard]] VkImage lookup(ImageHandle image) const;
        [[nodiscard]] VkImageView lookup(ImageViewHandle image_view) const;
        [[nodiscard]] VkSemaphore lookup(SemaphoreHandle semaphore) const;
        [[nodiscard]] VkFence lookup(FenceHandle fence) const;
        [[nodiscard]] VkDescriptorSet lookup(BindGroupHandle descriptor_set) const;
        [[nodiscard]] VkSampler lookup(SamplerHandle sampler) const;

        bool remove(PipelineLayoutHandle pipeline_layout);
        bool remove(PipelineHandle pipeline);
        bool remove(BufferHandle buffer);
        bool remove(ImageHandle image);
        bool remove(ImageViewHandle image_view);
        bool remove(SemaphoreHandle semaphore);
        bool remove(FenceHandle fence);
        bool remove(BindGroupLayoutHandle bind_group_layout);
        bool remove(BindGroupHandle descriptor_set);
        bool remove(SamplerHandle sampler);

    private:
        VkDevice device_;
        VmaAllocator allocator_;

        VulkanPool<UniqueVulkanDescriptorSetLayout> descriptor_set_layouts_;
        VulkanPool<UniqueVkPipelineLayout> pipeline_layouts_;
        VulkanPool<UniqueVkPipeline> pipelines_;
        VulkanPool<UniqueVulkanBuffer> buffers_;
        VulkanPool<UniqueVulkanImage> images_;
        VulkanPool<UniqueVkImageView> image_views_;
        VulkanPool<UniqueVkSemaphore> semaphores_;
        VulkanPool<UniqueVkFence> fences_;
        VulkanPool<UniqueVkDescriptorPool> descriptor_pools_;
        VulkanPool<UniqueVkDescriptorSet> descriptor_sets_;
        VulkanPool<UniqueVkSampler> samplers_;
    };
} // namespace orion
