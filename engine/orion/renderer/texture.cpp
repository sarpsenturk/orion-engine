#include "texture.hpp"

#include "orion/core/logger.hpp"
#include "orion/utils/scope_guard.hpp"

#include <stdexcept>

namespace orion
{
    TextureManager::TextureManager(gpu::Device* device)
        : m_device(device)
        , m_sampler(gpu::create_sampler(
              device,
              gpu::SamplerDesc{
                  .filter         = gpu::Filter::Linear,
                  .address_mode_u = gpu::SamplerAddressMode::ClampToEdge,
                  .address_mode_v = gpu::SamplerAddressMode::ClampToEdge,
                  .address_mode_w = gpu::SamplerAddressMode::ClampToEdge,
              }))
        , m_sampler_heap(gpu::create_sampler_heap(
              m_device,
              gpu::SamplerHeapDesc{.sampler_count = 1u}))
        , m_texture_heap(gpu::create_resource_heap(
              m_device,
              gpu::ResourceHeapDesc{
                  .sampled_image_count = max_texture_count,
                  .storage_image_count = 0u,
              }))
    {
        gpu::write_sampler(m_device, m_sampler_heap, 0u, m_sampler);
    }

    TextureManager::~TextureManager()
    {
        // Destroy remaining living textures
        for (const auto& texture : m_texture_slots) {
            gpu::destroy_image_view(m_device, texture.image_view);
            gpu::destroy_image(m_device, texture.image);
        }

        gpu::destroy_resource_heap(m_device, m_texture_heap);
        gpu::destroy_sampler_heap(m_device, m_sampler_heap);
        gpu::destroy_sampler(m_device, m_sampler);
    }

    Texture TextureManager::create_texture(const TextureDesc& desc)
    {
        if (m_texture_slots.size() >= max_texture_count) {
            throw std::runtime_error("TextureManager: max_texture_count exceeded");
        }

        // Create image
        auto* image = gpu::create_image(
            m_device,
            gpu::ImageDesc{
                .width  = desc.width,
                .height = desc.height,
                .format = gpu::Format::RGBA8_Srgb,
                .usage  = gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferDst,
            });
        const auto image_guard = ScopeFail{[image, this] { gpu::destroy_image(m_device, image); }};

        // Create image view
        auto* image_view = gpu::create_image_view(
            m_device,
            gpu::ImageViewDesc{
                .image             = image,
                .subresource_range = {
                    .base_mip_level   = 0u,
                    .level_count      = 1u,
                    .base_array_layer = 0u,
                    .layer_count      = 1u,
                },
            });
        const auto view_guard = ScopeFail{[image_view, this] { gpu::destroy_image_view(m_device, image_view); }};

        // Create texture handle and write to texture heap
        const auto handle = m_texture_slots.insert({image, image_view});
        const auto slot_guard = ScopeFail{[handle, this] { m_texture_slots.erase(handle); }};
        gpu::write_sampled_image(m_device, m_texture_heap, handle.index, image_view);

        return Texture{handle.to_uint64()};
    }

    void TextureManager::destroy_texture(Texture texture)
    {
        const auto handle = SlotMap<TextureData>::Handle::from_uint64(texture.handle);
        if (const auto* data = m_texture_slots.try_get(handle)) {
            gpu::destroy_image_view(m_device, data->image_view);
            gpu::destroy_image(m_device, data->image);
            m_texture_slots.erase(handle);
        } else {
            ORION_LOG_WARN(LogChannel::Renderer, "Trying to destroy Texture ({}) that is invalid", texture.handle);
        }
    }

    gpu::Image* TextureManager::get_image(Texture texture) const
    {
        const auto handle = SlotMap<TextureData>::Handle::from_uint64(texture.handle);
        const auto* data  = m_texture_slots.try_get(handle);
        return data ? data->image : nullptr;
    }
    gpu::ImageView* TextureManager::get_image_view(Texture texture) const
    {
        const auto handle = SlotMap<TextureData>::Handle::from_uint64(texture.handle);
        const auto* data  = m_texture_slots.try_get(handle);
        return data ? data->image_view : nullptr;
    }
} // namespace orion
