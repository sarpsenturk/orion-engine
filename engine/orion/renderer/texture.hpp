#pragma once

#include "orion/gpu/gpu.hpp"

#include "orion/utils/slot_map.hpp"

#include <cstdint>

namespace orion
{
    struct Texture {
        std::uint64_t handle = ~0ull;
    };

    struct TextureDesc {
        std::uint32_t width  = 0u;
        std::uint32_t height = 0u;
        // Required tightly packed RGBA8 sRGB pixels (width * height * 4 bytes).
        // Renderer::create_texture copies these bytes before returning.
        gpu::ByteSpan data = {};
    };

    class TextureManager
    {
    public:
        // Maximum number of textures allowed.
        static constexpr auto max_texture_count = 1'024u;

        explicit TextureManager(gpu::Device* device);
        TextureManager(const TextureManager&)            = delete;
        TextureManager& operator=(const TextureManager&) = delete;
        TextureManager(TextureManager&&)                 = delete;
        TextureManager& operator=(TextureManager&&)      = delete;
        ~TextureManager();

        Texture create_texture(const TextureDesc& desc);
        void destroy_texture(Texture texture);

        // Includes resources awaiting deferred destruction.
        [[nodiscard]] std::uint32_t texture_count() const noexcept { return static_cast<std::uint32_t>(m_texture_slots.size()); }

        // Owned image backing allocations in bytes, including pending destruction.
        [[nodiscard]] std::uint64_t allocation_bytes() const noexcept { return m_allocation_bytes; }

        [[nodiscard]] gpu::Image* get_image(Texture texture) const;
        [[nodiscard]] gpu::ImageView* get_image_view(Texture texture) const;
        [[nodiscard]] gpu::SamplerHeap* sampler_heap() const { return m_sampler_heap; }
        [[nodiscard]] gpu::ResourceHeap* resource_heap() const { return m_texture_heap; }

    private:
        struct TextureData {
            gpu::Image* image;
            gpu::ImageView* image_view;
        };

        gpu::Device* m_device;
        gpu::Sampler* m_sampler;
        gpu::SamplerHeap* m_sampler_heap;
        gpu::ResourceHeap* m_texture_heap;
        SlotMap<TextureData> m_texture_slots;
        std::uint64_t m_allocation_bytes = 0ull;
    };
} // namespace orion
