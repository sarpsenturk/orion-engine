#pragma once

#include "orion-renderapi/defs.h"
#include "orion-renderapi/device_resource.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace orion
{
    enum class TextureType {
        Tex2D,
    };

    struct TextureInfo {
        std::uint32_t width;
        std::uint32_t height;
        TextureType type;
        Format format;
        AddressMode u;
        AddressMode v;
        AddressMode w;
        Filter filter;
        bool host_visible;
    };

    ImageType to_image_type(TextureType type);
    ImageViewType to_image_view_type(TextureType type);

    class Texture
    {
    public:
        Texture(UniqueImage image, UniqueImageView image_view, UniqueSampler sampler, TextureInfo info);

        [[nodiscard]] ImageHandle image() const { return image_.get(); }
        [[nodiscard]] ImageViewHandle image_view() const { return image_view_.get(); }
        [[nodiscard]] SamplerHandle sampler() const { return sampler_.get(); }

    private:
        UniqueImage image_;
        UniqueImageView image_view_;
        UniqueSampler sampler_;
        TextureInfo info_;
    };
} // namespace orion
