#pragma once

#include "orion-core/filesystem.h"

#include "orion-renderapi/device_resource.h"
#include "orion-renderapi/render_command.h"

#include "orion-utils/expected.h"

#include <vector>

// Forward declare
namespace spdlog
{
    class logger;
}

namespace orion
{
    struct TextureInfo {
        std::int32_t width;
        std::int32_t height;
        std::int32_t channels;

        [[nodiscard]] std::size_t size_bytes() const noexcept { return width * height * channels; }
    };

    class Texture
    {
    public:
        Texture(UniqueImage image, UniqueImageView image_view, TextureInfo info);

        [[nodiscard]] ImageHandle image() const noexcept { return image_.get(); }
        [[nodiscard]] ImageViewHandle image_view() const noexcept { return image_view_.get(); }
        [[nodiscard]] std::int32_t width() const noexcept { return info_.width; }
        [[nodiscard]] std::int32_t height() const noexcept { return info_.height; }
        [[nodiscard]] std::int32_t channels() const noexcept { return info_.channels; }
        [[nodiscard]] const TextureInfo& info() const noexcept { return info_; }

    private:
        UniqueImage image_;
        UniqueImageView image_view_;
        TextureInfo info_;
    };

    // Forward declare
    class RenderDevice;

    // Texture index type to be used for descriptors
    using texture_index_t = int;

    namespace textures
    {
        static constexpr auto missing = 0;
        static constexpr auto white = 1;
    } // namespace textures

    struct TextureCreateSuccess {
        const Texture* texture;
        texture_index_t index;
    };

    using TextureCreateResult = expected<TextureCreateSuccess, const char*>;

    class TextureManager
    {
    public:
        static constexpr auto max_samplers = 4096;
        static constexpr auto max_textures = 4096;
        static constexpr auto sampler_binding = 0;
        static constexpr auto texture_binding = 1;

        explicit TextureManager(RenderDevice* device);

        TextureCreateResult add_texture(const unsigned char* data, const TextureInfo& info);
        [[nodiscard]] TextureCreateResult load_from_file(const FilePath& path);

        [[nodiscard]] const Texture* get(texture_index_t index) const;

        void set_sampler(SamplerHandle sampler_handle);
        void set_sampler_default();

        [[nodiscard]] DescriptorLayoutHandle descriptor_layout() const noexcept { return descriptor_layout_.get(); }
        [[nodiscard]] DescriptorHandle descriptor() const noexcept { return descriptor_; }

    private:
        static spdlog::logger* logger();

        [[nodiscard]] UniqueDescriptorPool create_descriptor_pool() const;
        [[nodiscard]] UniqueDescriptorLayout create_descriptor_layout() const;
        [[nodiscard]] DescriptorHandle create_descriptor() const;
        [[nodiscard]] UniqueSampler create_default_sampler() const;

        [[nodiscard]] Texture make_texture(const unsigned char* data, TextureInfo info);
        [[nodiscard]] void set_texture(texture_index_t index, const Texture& texture);

        RenderDevice* device_;
        UniqueDescriptorPool descriptor_pool_;
        UniqueDescriptorLayout descriptor_layout_;
        DescriptorHandle descriptor_;
        CommandAllocatorPtr command_allocator_;
        CommandListPtr command_list_;
        std::vector<Texture> textures_;
        UniqueSampler default_sampler_;
    };
} // namespace orion
