#include "orion-renderer/texture.h"

#include <utility>

namespace orion
{
    Texture::Texture(UniqueImage image, UniqueImageView image_view, UniqueSampler sampler, TextureInfo info)
        : image_(std::move(image))
        , image_view_(std::move(image_view))
        , sampler_(std::move(sampler))
        , info_(info)
    {
    }

    ImageType to_image_type(TextureType type)
    {
        switch (type) {
            case TextureType::Tex2D:
                return ImageType::Image2D;
        }
    }

    ImageViewType to_image_view_type(TextureType type)
    {
        switch (type) {
            case TextureType::Tex2D:
                return ImageViewType::View2D;
        }
    }
} // namespace orion
