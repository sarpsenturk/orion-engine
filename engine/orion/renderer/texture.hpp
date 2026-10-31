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

        [[nodiscard]] gpu::Image* get_image(Texture texture) const;
        [[nodiscard]] gpu::ImageView* get_image_view(Texture texture) const;

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
    };
} // namespace orion
