#pragma once

#include "orion-core/filesystem.h"

#include "orion-renderapi/device_resource.h"
#include "orion-renderapi/render_command.h"

#include "orion-utils/expected.h"

#include <vector>

namespace orion
{
    struct TextureDesc {
        const unsigned char* data;
        int width;
        int height;
        int channels;

        [[nodiscard]] std::size_t size_bytes() const noexcept { return width * height * channels; }
    };

    class Texture
    {
    public:
        Texture(UniqueImage image, UniqueImageView image_view);

        [[nodiscard]] ImageHandle image() const noexcept { return image_.get(); }
        [[nodiscard]] ImageViewHandle image_view() const noexcept { return image_view_.get(); }

    private:
        UniqueImage image_;
        UniqueImageView image_view_;
    };

    // Forward declare
    class RenderDevice;

    using TextureLoadResult = expected<Texture, const char*>;

    class TextureLoader
    {
    public:
        explicit TextureLoader(RenderDevice* device);

        [[nodiscard]] TextureLoadResult load_from_memory(const TextureDesc& desc);
        [[nodiscard]] TextureLoadResult load_from_file(const FilePath& path);

    private:
        RenderDevice* device_;
        CommandAllocatorPtr command_allocator_;
        CommandListPtr command_list_;
    };

    // Texture index type to be used for descriptors
    using texture_index_t = int;

    namespace textures
    {
        static constexpr auto missing = 0;
        static constexpr auto white = 1;
    } // namespace textures

    class TextureArray
    {
    public:
        static constexpr auto max_samplers = 1;
        static constexpr auto max_textures = 4096;
        static constexpr auto sampler_binding = 0;
        static constexpr auto texture_binding = 1;

        explicit TextureArray(RenderDevice* device);

        void set(texture_index_t index, const Texture& texture);
        texture_index_t add(const Texture& texture);

        void set_all(const Texture& texture);

        void set_sampler(SamplerHandle sampler_handle);
        void set_sampler_default();

        [[nodiscard]] DescriptorLayoutHandle descriptor_layout() const noexcept { return descriptor_layout_.get(); }
        [[nodiscard]] DescriptorHandle descriptor() const noexcept { return descriptor_; }

    private:
        [[nodiscard]] UniqueDescriptorPool create_descriptor_pool() const;
        [[nodiscard]] UniqueDescriptorLayout create_descriptor_layout() const;
        [[nodiscard]] DescriptorHandle create_descriptor() const;
        [[nodiscard]] UniqueSampler create_default_sampler() const;


        RenderDevice* device_;
        UniqueDescriptorPool descriptor_pool_;
        UniqueDescriptorLayout descriptor_layout_;
        DescriptorHandle descriptor_;
        UniqueSampler default_sampler_;
        texture_index_t texture_index_ = 1;
    };
} // namespace orion
